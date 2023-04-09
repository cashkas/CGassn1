#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/vec3.hpp>
#include <vector>
#include <ctime>


#include <list>
#include <vector>

#define PI = 3.14159265f;
const float WINDOW_WIDTH = 800.0f;
const float WINDOW_HEIGHT = 600.0f;
const float WINDOW_RATIO = WINDOW_WIDTH / WINDOW_HEIGHT;
const float GROUND_Y = -0.5f;
const float MAP_VELOCITY = -0.015f;
float HOLE_WIDTH = 0.5f;
float viewportX = 1.0f;
float GRAVITY = -0.001f;
float JUMP_SPEED = 0.03f;
int score = 0; // 점수
bool gameOver = false; // 게임 종료 여부
char scoreArr[10];
char gameOverMsg[] = "Game Over";
bool hasGameOver = false;
int currentStarPattern = 0;
bool isFalling = false;
bool isStart = false;

enum class JumpState {
	ON_GROUND,		// 땅을 밟은 상태
	IN_AIR,			// 공중에 있는 상태 (점프/낙하)
	AERIAL_JUMP,	// 공중 점프
	NO_JUMP			// 점프 불가
};
enum class State {
	IDLE,			// 기본 상태
	STUCK,			// 지형지물에 막힘
	RECOVERY		// 기본 위치로 복구
};
class Object {
private:
	float x;
	float y;
	float width;
	float height;

public:
	Object(float x_, float y_, float width_ = 1.0f, float height_ = 1.0f) :
		x(x_), y(y_), width(width_), height(height_)
	{}
	virtual float getX() {
		return x;
	}
	virtual float getY() {
		return y;
	}
	float getRight() { return getX() + width; }
	float getTop() { return getY() + height; }
	void setX(float x_) {
		x = x_;
	}
	void setObject(float x_, float y_, float width_, float height_)
	{
		x = x_;
		y = y_;
		width = width_;
		height = height_;
	}
	void setY(float y_) {
		y = y_;
	}
	void setCoord(float x_, float y_) {
		x = x_;
		y = y_;
	}
	void setWidth(float width_) {
		width = width_;
	}
	void setHeight(float height_) {
		height = height_;
	}
	float getWidth() {
		return width;
	}
	float getHeight() {
		return height;
	}
	virtual bool isCollided(Object object) // 매개변수 object와 충돌 여부 반환
	{
		bool xOverlap = getX() + getWidth() >= object.getX() && object.getX() + object.getWidth() >= getX();
		bool yOverlap = getY() + getHeight() >= object.getY() && object.getY() + object.getHeight() >= getY();
		return xOverlap && yOverlap;
	}

	virtual bool isCollided(Object object, float margin) // 매개변수 object와 충돌 여부 반환, margin : 충돌 범위 추가 넓힐 때 사용
	{
		bool xOverlap = getX() + getWidth() + margin >= object.getX() && object.getX() + object.getWidth() + margin >= getX();
		bool yOverlap = getY() + getHeight() + margin >= object.getY() && object.getY() + object.getHeight() + margin * WINDOW_RATIO >= getY();
		return xOverlap && yOverlap;
	}
};



class MovingObject : public Object {
private:
	float x_viewport; // viewportX 기준 x 축 방향으로 어느 위치에 있는지
public:
	MovingObject(float x_, float y_, float width_, float height_)
		:Object(x_, y_, width_, height_), x_viewport(x_ - viewportX)
	{}
	float getX() override
	{
		__super::setX(viewportX + x_viewport);
		return __super::getX();
	}
	void setXViewport(float x_viewport_)
	{
		x_viewport = x_viewport_;
		setX(viewportX + x_viewport);
	}
};

class Terrain : public MovingObject {
private:
	bool isHole;
public:
	Terrain(float x_, float y_, float width_, float height_, bool isHole_ = false)
		:MovingObject(x_, y_, width_, height_), isHole(isHole_)
	{}
	bool isGround()
	{
		return !isHole;
	}
	void setIsHole(bool isHole_)
	{
		isHole = isHole_;
	}
	float getSurfaceY()
	{
		return getY() + getHeight();
	}

};

class Star :public MovingObject {
private:
	bool collected;
	float outerRadius = 0.04f;
	float innerRadius = 0.03f;

public:
	Star(float x_, float y_, bool collected_ = true)
		: MovingObject(x_ - outerRadius, y_ - outerRadius * WINDOW_RATIO, 2 * outerRadius, 2 * outerRadius * WINDOW_RATIO),
		collected(false)
	{
	}
	void enableStar()
	{
		collected = false;
	}
	void disableStar()
	{
		collected = true;
	}
	float getCenterX()
	{
		return getX() + outerRadius;
	}
	float getCenterY()
	{
		return getY() + outerRadius;
	}
	bool isCollected()
	{
		return collected;
	}
};
//class Mushroom : public MovingObject
//{
//private:
//	Terrain terrain;
//public:
//	Mushroom(float terrain_x, float width, float height, Terrain terrain_): // terrain_x : terrain의 x좌표 기준 어느 위치에 있는 지
//		
//};
class Fireball : public MovingObject {
private:
	float speed;
	float radius;
public:
	Fireball(float x_, float y_, float radius_, float speed_) :
		MovingObject(x_ - radius_, y_ - radius_ * WINDOW_RATIO, 2 * radius, 2 * radius * WINDOW_RATIO),
		speed(speed_), radius(radius_)
	{}
	void updatePosition(float viewPort)
	{
		setX(getX() - speed);
	}
	float getRadius()
	{
		return radius;
	}
	float getSpeed()
	{
		return speed;
	}
};

class Character : public Object {
private:
	float jumpSpeed = 0.03f;
	JumpState characterJumpState;
public:
	Character(float x_, float y_, float width_, float height_) :
		Object(x_, y_, width_, height_), isJumping(false), isDoubleJumping(false), characterJumpState(JumpState::ON_GROUND)
	{}

	bool isJumping;
	bool isDoubleJumping;
	float getJumpSpeed()
	{
		return jumpSpeed;
	}
	void setJumpSpeed(float jumpSpeed_)
	{
		jumpSpeed = jumpSpeed_;
	}
	void jump()
	{
		std::cout << "jump" << std::endl;
		if (characterJumpState == JumpState::ON_GROUND)
		{
			characterJumpState = JumpState::IN_AIR;
			setJumpSpeed(0.03f);
		}
		else if (characterJumpState == JumpState::IN_AIR)
		{
			characterJumpState = JumpState::AERIAL_JUMP;
			setJumpSpeed(0.03f);
		}
	}


	JumpState getJumpState()
	{
		return characterJumpState;
	}
	void setJumpState(JumpState state)
	{
		characterJumpState = state;
	}
	bool isCollided(Object object) override// 매개변수 object와 충돌 여부 반환
	{
		bool xOverlap = getX() + getWidth() * 0.5f >= object.getX() && object.getX() + object.getWidth() >= getX();
		bool yOverlap = getY() + getWidth() * 0.6f + 0.3f >= object.getY() && object.getY() + object.getHeight() >= getY();
		return xOverlap && yOverlap;
	}

	bool isCollided(Object object, float margin) override // 매개변수 object와 충돌 여부 반환, margin : 충돌 범위 추가 넓힐 때 사용
	{
		bool xOverlap = getX() + getWidth() + margin >= object.getX() && object.getX() + object.getWidth() + margin >= getX();
		bool yOverlap = getY() + getHeight() + margin >= object.getY() && object.getY() + object.getHeight() + margin * WINDOW_RATIO >= getY();
		return xOverlap && yOverlap;
	}
	bool isOnGround(std::list<Terrain> terrainList_)
	{
		for (std::list<Terrain>::iterator it = terrainList_.begin(); it != terrainList_.end(); it++)
		{
			if (it->isGround() && (getX() + getWidth() >= it->getX()) && (it->getX() + it->getWidth() >= getX())) // x overlap
			{
				if (isCollided(*it) && (it->getTop() >= getY() - 0.001f)) // y overlap
				{
					std::cout << "ground height: " << it->getHeight() << std::endl;
					return true;
				}
			}
		}
		return false;
	}
	float surfaceY(std::list<Terrain> terrainList_)
	{
		if (!isOnGround(terrainList_)) return 0.0f;
		else
		{
			for (std::list<Terrain>::iterator it = terrainList_.begin(); it != terrainList_.end(); it++)
			{
				if (it->isGround() && (getX() + getWidth() >= it->getX()) && (it->getX() + it->getWidth() >= getX())) // x overlap
				{
					if (isCollided(*it) && (it->getTop() >= getY() - 0.01f)) // y overlap
					{
						return it->getSurfaceY();
					}
				}
			}
		}

	}
};

Character character = Character(-0.5f, GROUND_Y, 0.1f, 0.1f * WINDOW_RATIO);
Fireball fb = Fireball(1.0f, GROUND_Y + 0.2f, 0.05f, 0.01f);

std::list<Star> starList;
std::list<Star> enabledStarList;
std::list<Terrain> terrainList;



//초기화
void init()
{
	std::cout << "init" << std::endl;
	glLoadIdentity();

	Star star1 = Star(0.2f, GROUND_Y + 0.1f, false);
	Star star2 = Star(0.4f, GROUND_Y + 0.1f, false);
	Star star3 = Star(0.6f, GROUND_Y + 0.1f, false);
	starList.push_back(star1);
	starList.push_back(star2);
	starList.push_back(star3);
	starList.push_back(Star(0.2f, GROUND_Y + 0.2f, false));
	starList.push_back(Star(0.4f, GROUND_Y + 0.2f, false));
	starList.push_back(Star(0.6f, GROUND_Y + 0.2f, false));
	starList.push_back(Star(0.2f, GROUND_Y + 0.3f, false));
	starList.push_back(Star(0.4f, GROUND_Y + 0.3f, false));
	starList.push_back(Star(0.6f, GROUND_Y + 0.3f, false));

	Terrain terrain1 = Terrain(-3.0f, -1.0f, 2.0f, 0.6f);
	Terrain terrain2 = Terrain(-1.0f, -1.0f, 1.5f, 0.4f);
	Terrain terrain3 = Terrain(0.5f, -1.0f, 0.5f, 0.4f, true);
	Terrain terrain4 = Terrain(1.0f, -1.0f, 2.0f, 0.5f);
	Terrain terrain5 = Terrain(3.0f, -1.0f, 2.0f, 0.6f);
	terrainList.push_back(terrain1);
	terrainList.push_back(terrain2);
	terrainList.push_back(terrain3);
	terrainList.push_back(terrain4);
	terrainList.push_back(terrain5);
}

//draw 함수
void drawStar(Star& star, float viewport)
{
	float outer = 0.04f, inner = 0.03f;
	float rad = 3.14159265 / 180.0;
	glPushMatrix();

	glColor3f(0, 0, 0);
	if (!star.isCollected()) // star를 획득하지 않은 경우에만 화면에 표시.
	{
		glBegin(GL_POLYGON);
		for (int i = 0; i < 5; i++)
		{
			glVertex2f(star.getCenterX() + 0.04f * cos((18.0f + 72.0f * i) * rad), star.getCenterY() + 0.04f * sin((18.0f + 72.0f * i) * rad) * WINDOW_RATIO);
			glVertex2f(star.getCenterX() + 0.045f * cos((54.0f + 72.0f * i) * rad), star.getCenterY() + 0.045f * sin((54.0f + 72.0f * i) * rad) * WINDOW_RATIO);
		}
		glEnd();
	}

	glColor3f(1.0f, 1.0f, 0.0f);
	if (!star.isCollected()) // star를 획득하지 않은 경우에만 화면에 표시.
	{
		glBegin(GL_POLYGON);
		for (int i = 0; i < 5; i++)
		{
			glVertex2f(star.getCenterX() + inner * cos((18.0f + 72.0f * i) * rad), star.getCenterY() + inner * sin((18.0f + 72.0f * i) * rad) * WINDOW_RATIO);
			glVertex2f(star.getCenterX() + outer * cos((54.0f + 72.0f * i) * rad), star.getCenterY() + outer * sin((54.0f + 72.0f * i) * rad) * WINDOW_RATIO);
		}
		glEnd();
	}

	glPopMatrix();
}

void drawFireball(Fireball fireball, float viewport)
{
	glPushMatrix();
	glColor3f(1.0f, 0, 0);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 360; i++)
	{
		float theta = i * 3.14159265 / 180.0f;

		glVertex2f(fireball.getX()
			+ fireball.getRadius() * cos(theta),
			fireball.getY() + fireball.getRadius() * sin(theta) * WINDOW_RATIO);
	}
	glEnd();
	glPopMatrix();
}

void drawTerrain(Terrain terrain, float viewport)
{
	glPushMatrix();
	if (terrain.isGround()) glColor3f(0, 1, 0); // 땅: 초록색
	else glColor3f(0, 0, 0); // 구멍: 검정

	glBegin(GL_QUADS); // 땅 그리기
	glVertex2f(terrain.getX(), terrain.getY() + terrain.getHeight());
	glVertex2f(terrain.getX() + terrain.getWidth(), terrain.getY() + terrain.getHeight());
	glVertex2f(terrain.getX() + terrain.getWidth(), terrain.getY());
	glVertex2f(terrain.getX(), terrain.getY());
	glEnd();
	glPopMatrix();
}


void drawCharacter(Character character, int frame)
{
	if (isStart) {
		if (frame == 0) {
			character.setY(character.getY() - 0.01f);
		}
		if (frame == 1) {
			character.setY(character.getY() + 0.01f);
		}
		if (frame == 2) {
			character.setY(character.getY() - 0.01f);
		}
		if (frame == 3) {
			character.setY(character.getY() + 0.01f);
		}
	}
	else
		isStart = true;
	//머리
	glPushMatrix();
	glTranslatef(character.getX(), character.getY() + 0.3f, 0.0f);
	glColor3f(1, 0, 1);
	glScalef(0.5f, 0.6f, 0.0f);
	glutSolidCube(character.getWidth());
	glPopMatrix();

	//목
	glPushMatrix();
	glTranslatef(character.getX() - 0.01f, character.getY() + 0.25f, 0.0f);
	glColor3f(1, 0, 1);
	glScalef(0.25f, 0.5f, 0.0f);
	glutSolidCube(character.getWidth());
	glPopMatrix();

	//팔
	float upperarmAngles[4] = { -30.0f, 0.0f, 30.0f, 0.0f };
	float l_lowerarmAngles[4] = { 15.0f, 0.0f, 30.0f, 0.0f };
	float r_lowerarmAngles[4] = { 45.0f, 0.0f, 15.0f, 0.0f };

	//왼팔
	//upper
	glPushMatrix();
	glTranslatef(character.getX(), character.getY() + 0.2f, 0.0f);
	glColor3f(0.9, 0, 1);
	glRotatef(upperarmAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.001f, 0.0f);
	glScalef(0.3f, 0.6f, 0.0f);
	glutSolidCube(character.getWidth());

	//lower
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(l_lowerarmAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	glScalef(1.0f, 1.0f, 0.0f);
	glutSolidCube(character.getWidth());

	//hand
	glTranslatef(0.0f, -0.08f, 0.0f);
	glScalef(0.5f, 0.5f, 0.0f);
	glutSolidCube(character.getWidth());
	glPopMatrix();

	//발
	float upperlegAngles[4] = { 30.0f, 0.0f, -30.0f, 0.0f };
	float l_lowerlegAngles[4] = { -30.0f, 0.0f, 0.0f, 0.0f };
	float r_lowerlegAngles[4] = { 0.0f, 0.0f, -30.0f, 0.0f };

	//왼발
	//upper
	glPushMatrix();
	glTranslatef(character.getX(), character.getY() + 0.15f, 0.0f);
	glColor3f(0.9, 0, 1);
	glRotatef(upperlegAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.05f, 0.0f);
	glScalef(0.3f, 0.7f, 0.0f);
	glutSolidCube(character.getWidth());

	//lower
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(l_lowerlegAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	glScalef(0.8f, 1.0f, 0.0f);
	glutSolidCube(character.getWidth());

	//foot
	glTranslatef(0.0f, -0.05f, 0.0f);
	glScalef(1.0f, 0.4f, 0.0f);
	glutSolidCube(character.getWidth());
	glPopMatrix();

	//몸
	glPushMatrix();
	glTranslatef(character.getX(), character.getY() + 0.2f, 0.0f);
	glColor3f(1, 0, 1);
	glScalef(0.5f, 1.0f, 0.0f);
	glutSolidCube(character.getWidth());
	glPopMatrix();

	//골판
	glPushMatrix();
	glTranslatef(character.getX(), character.getY() + 0.15f, 0.0f);
	glColor3f(1, 0, 1);
	glScalef(0.45f, 0.5f, 0.0f);
	glutSolidCube(character.getWidth());
	glPopMatrix();

	//오른팔
	//upper
	glPushMatrix();
	glTranslatef(character.getX(), character.getY() + 0.2f, 0.0f);
	glColor3f(0.9, 0, 1);
	glRotatef(-upperarmAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.001f, 0.0f);
	glScalef(0.3f, 0.6f, 0.0f);
	glutSolidCube(character.getWidth());

	//lower
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(r_lowerarmAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	glScalef(1.0f, 1.0f, 0.0f);
	glutSolidCube(character.getWidth());

	//hand
	glTranslatef(0.0f, -0.08f, 0.0f);
	glScalef(0.5f, 0.5f, 0.0f);
	glutSolidCube(character.getWidth());
	glPopMatrix();

	//오른발
	//upper
	glPushMatrix();
	glTranslatef(character.getX(), character.getY() + 0.15f, 0.0f);
	glColor3f(0.9, 0, 1);
	glRotatef(-upperlegAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.05f, 0.0f);
	glScalef(0.3f, 0.7f, 0.0f);
	glutSolidCube(character.getWidth());

	//lower
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(r_lowerlegAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	glScalef(0.8f, 1.0f, 0.0f);
	glutSolidCube(character.getWidth());

	//foot
	glTranslatef(0.0f, -0.05f, 0.0f);
	glScalef(1.0f, 0.4f, 0.0f);
	glutSolidCube(character.getWidth());
	glPopMatrix();
}

void drawScore()
{
	glPushMatrix();

	glColor3f(0.0f, 0.0f, 0.0f);
	sprintf_s(scoreArr, "Score: %d", score);
	int len = strlen(scoreArr);
	glRasterPos2f(-0.9f, 0.9f);
	for (int i = 0; i < len; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, scoreArr[i]);
	}
	glPopMatrix();

}

void updateCharacter()
{

	switch (character.getJumpState())
	{
	case JumpState::ON_GROUND:
		std::cout << "on ground" << std::endl;
		break;
	case JumpState::IN_AIR:
		std::cout << "in air" << std::endl;
		break;
	case JumpState::AERIAL_JUMP:
		std::cout << "aerial" << std::endl;
		break;
	}

	// 공중에 있을 때
	if (character.getJumpState() != JumpState::ON_GROUND)
	{
		// 중력 가속
		character.setY(character.getY() + character.getJumpSpeed());
		character.setJumpSpeed(character.getJumpSpeed() + GRAVITY);

		if (character.getJumpState() == JumpState::AERIAL_JUMP)
		{
			character.setJumpState(JumpState::NO_JUMP);
		}


		// 땅에 닿으면
		if (character.isOnGround(terrainList))
		{
			std::cout << "on ground" << std::endl;

			character.setY(character.surfaceY(terrainList));
			character.setJumpState(JumpState::ON_GROUND);
		}


	}
	else // 땅에 잇을 때
	{
		if (!character.isOnGround(terrainList)) // 땅을 밟지 않은 경우 (떨어져야 하는 경우)
		{
			character.setJumpState(JumpState::IN_AIR);
			character.setJumpSpeed(0.0f);
		}

		// 땅에 닿으면
		if (character.isOnGround(terrainList))
		{
			std::cout << "on ground" << std::endl;

			character.setY(character.surfaceY(terrainList));
			character.setJumpState(JumpState::ON_GROUND);
		}
	}







}
// 게임 로직
void update(int value)
{
	viewportX += MAP_VELOCITY;

	if (viewportX < -3.0f)
	{
		//srand(time(NULL)); // 랜덤 요소 추가
		viewportX = 3.0f;// + ((double)rand() / RAND_MAX);

		enabledStarList.clear();

		srand(time(NULL));
		int count = 0;
		int random = rand() % starList.size();

		for (std::list<Star>::iterator it = starList.begin(); it != starList.end(); it++) {
			if ((random - count <= 1) && (random - count >= -1))
			{
				(*it).enableStar();
				enabledStarList.push_back(*it);
			}
			count++;
		}
	}

	fb.updatePosition(viewportX);
	updateCharacter();

	// 맵의 다른 요소들 업데이트 (fireball, stars 등)


	/*충돌 체크*/
	for (std::list<Star>::iterator it = enabledStarList.begin(); it != enabledStarList.end(); it++) {
		drawStar(*it, viewportX);
		if (character.isCollided(*it, 0.01f))
		{
			if ((*it).isCollected() == false) {
				score++;
				(*it).disableStar();
			}

		}
	}

	if (character.isCollided(fb) || character.getY() < -1.0f) {
		gameOver = true;
	}

	glutTimerFunc(16, update, 0); // ~60fps
	glutPostRedisplay();
}


void keyboard(unsigned char key, int x, int y) // int x, y: 마우스 커서 위치
{
	switch (key) {
	case 27: // ESC
		exit(0);
		break;
	case 32: // 스페이스바
		character.jump();
		break;
	}
	//glutPostRedisplay();
}

void drawTerrains()
{
	for (std::list<Terrain>::iterator it = terrainList.begin(); it != terrainList.end(); it++)
	{
		drawTerrain(*it, viewportX);
	}
}
void drawStars()
{
	std::cout << enabledStarList.size() << std::endl;
	for (std::list<Star>::iterator it = enabledStarList.begin(); it != enabledStarList.end(); it++) {
		drawStar(*it, viewportX);
	}

}
// 렌더링
void display(void) {

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT); //OpenGL의 프레임 버퍼에 있는 모든 픽셀들의 색상 값이 clear color로 설정됨


	// 맵 그리기 (terrain, fireball, stars)
	drawFireball(fb, viewportX);
	drawTerrains();
	//drawTerrain(viewportX);
	drawScore();


	// 캐릭터 그리기
	static int frame = 0;
	static int frame2 = 0;
	drawCharacter(character, frame);

	frame2++;
	frame = (frame2 / 8) % 4;



	drawStars();

	// 게임 종료시 점수 표시
	if (gameOver) {
		// 점수 표시
		glRasterPos2f(-0.2f, 0.0f);
		glColor3f(0.0f, 0.0f, 0.0f);

		for (int i = 0; i < strlen(scoreArr); i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, scoreArr[i]);
		}
		glRasterPos2f(-0.2f, 0.1f);
		for (int i = 0; i < strlen(gameOverMsg); i++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, gameOverMsg[i]);
		}
	}

	// 그리기 완료 후 화면 업데이트
	if (!hasGameOver)
		glutSwapBuffers();
	if (gameOver)
		hasGameOver = true;
}


int main(int argc, char** argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // 더블 버퍼링 모드
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT); // 윈도우 사이즈
	glutCreateWindow("Wind Runner"); // 윈도우 생성
	init();

	glutKeyboardFunc(keyboard);


	glutTimerFunc(0, update, 0);
	glutDisplayFunc(display);

	glewInit();

	glutMainLoop();
	return 0;
}