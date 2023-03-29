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
class Star {
public:
	Star(float x_, float height_, bool collected_ = true) :
		x(x_), height(height_), collected(false)
	{}
	float x;
	float height; // GROUND_Y 기준 높이
	bool collected;


};

class Fireball {
public:
	Fireball(float x_, float height_, float radius_, float speed_) :
		x(x_), height(height_), radius(radius_), speed(speed_)
	{}
	float x;
	float height; // GROUND_Y 기준 높이
	float radius;
	float speed;
	float windowX;
	void updatePosition(float viewPort)
	{
		windowX = x + speed * viewPort;
	}
};

class Character {
public:
	Character(float x_, float y_, float width_, float height_) :
		x(x_), y(y_), width(width_), height(height_), isJumping(false), isDoubleJumping(false)
	{}
	float x;
	float y;
	float width;
	float height;
	bool isJumping;
	bool isDoubleJumping;
};

Character character = Character(-0.5f, GROUND_Y, 0.1f, 0.1f);
Fireball fb = Fireball(0.0f, 0.1f, 0.05f, 1.5f);
Star star = Star(0.0f, 0.15f);
std::list<Star> starList;
std::list<Star> enabledStarList;



//초기화
void init()
{
	glLoadIdentity();

	Star star1 = Star(0.2f, 0.1f, false);
	Star star2 = Star(0.4f, 0.1f, false);
	Star star3 = Star(0.6f, 0.1f, false);
	starList.push_back(star1);
	starList.push_back(star2);
	starList.push_back(star3);
	starList.push_back(Star(0.2f, 0.2f, false));
	starList.push_back(Star(0.4f, 0.2f, false));
	starList.push_back(Star(0.6f, 0.2f, false));
	starList.push_back(Star(0.2f, 0.3f, false));
	starList.push_back(Star(0.4f, 0.3f, false));
	starList.push_back(Star(0.6f, 0.3f, false));


}



//draw 함수
void drawStar(Star& star, float viewport)
{
	float outer = 0.04f, inner = 0.03f;
	GLfloat rad = 3.14159265 / 180.0f;
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 0.0f);
	if (!star.collected) // star를 획득하지 않은 경우에만 화면에 표시.
	{
		glBegin(GL_POLYGON);
		for (int i = 0; i < 5; i++)
		{
			glVertex2f(star.x + viewport + inner * cos((18.0f + 72.0f * i) * rad), GROUND_Y + star.height + inner * sin((18.0f + 72.0f * i) * rad) * (GLfloat)WINDOW_WIDTH / WINDOW_HEIGHT);
			glVertex2f(star.x + viewport + outer * cos((54.0f + 72.0f * i) * rad), GROUND_Y + star.height + outer * sin((54.0f + 72.0f * i) * rad) * (GLfloat)WINDOW_WIDTH / WINDOW_HEIGHT);
		}
	}
	glEnd();

	glPopMatrix();
}

void drawFireball(Fireball fireball, float viewport)
{
	glPushMatrix();
	glColor3f(1.0f, 0, 0);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 360; i++)
	{
		GLfloat theta = i * 3.14159265 / 180.0f;

		glVertex2f(fireball.windowX + fireball.radius * cos(theta),
			fireball.height + GROUND_Y + fireball.radius * sin(theta) * (GLfloat)WINDOW_WIDTH / WINDOW_HEIGHT);
	}
	glEnd();
	glPopMatrix();
}
void drawTerrain()
{
	glPushMatrix();
	glColor3f(0, 1, 0);
	glBegin(GL_QUADS); // 땅 그리기
	glVertex2f(-1.0f, GROUND_Y);
	glVertex2f(1.0f, GROUND_Y);
	glVertex2f(1.0f, -1.0f);
	glVertex2f(-1.0f, -1.0f);
	glEnd();
	glPopMatrix();
}
void drawHole(float viewPort)
{
	glPushMatrix();
	glColor3f(0, 0, 0);
	glBegin(GL_QUADS); // 함정 그리기
	glVertex2f(viewPort, -1.0f);
	glVertex2f(viewPort, GROUND_Y);
	glVertex2f(viewPort + HOLE_WIDTH, -0.5f);
	glVertex2f(viewPort + HOLE_WIDTH, -1.0f);
	glEnd();
	glPopMatrix();
}

void drawCharacter(Character character, int frame)
{
   //머리
	glPushMatrix();
	glTranslatef(character.x, character.y + 0.3f, 0.0f);
	glColor3f(1, 0, 1);
	glScalef(0.5f, 0.6f, 0.0f);
	glutSolidCube(character.width);
	glPopMatrix();

	//목
	glPushMatrix();
	glTranslatef(character.x - 0.01f, character.y + 0.25f, 0.0f);
	glColor3f(1, 0, 1);
	glScalef(0.25f, 0.5f, 0.0f);
	glutSolidCube(character.width);
	glPopMatrix();

	//팔
	float upperarmAngles[4] = { -30.0f, 0.0f, 30.0f, 0.0f };
	float l_lowerarmAngles[4] = { 15.0f, 0.0f, 30.0f, 0.0f };
	float r_lowerarmAngles[4] = { 45.0f, 0.0f, 15.0f, 0.0f };
	
	//왼팔
	//upper
	glPushMatrix();
	glTranslatef(character.x, character.y + 0.2f, 0.0f);
	glColor3f(0.9, 0, 1);
	glRotatef(upperarmAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.001f, 0.0f);
	glScalef(0.3f, 0.6f, 0.0f);
	glutSolidCube(character.width);

	//lower
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(l_lowerarmAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	glScalef(1.0f, 1.0f, 0.0f);
	glutSolidCube(character.width);

	//hand
	glTranslatef(0.0f, -0.08f, 0.0f);
	glScalef(0.5f, 0.5f, 0.0f);
	glutSolidCube(character.width);
	glPopMatrix();

	//발
	float upperlegAngles[4] = { 30.0f, 0.0f, -30.0f, 0.0f };
	float l_lowerlegAngles[4] = { -30.0f, 0.0f, 0.0f, 0.0f };
	float r_lowerlegAngles[4] = { 0.0f, 0.0f, -30.0f, 0.0f };

	//왼발
	//upper
	glPushMatrix();
	glTranslatef(character.x, character.y + 0.15f, 0.0f);
	glColor3f(0.9, 0, 1);
	glRotatef(upperlegAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.05f, 0.0f);
	glScalef(0.3f, 0.7f, 0.0f);
	glutSolidCube(character.width);

	//lower
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(l_lowerlegAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	glScalef(0.8f, 1.0f, 0.0f);
	glutSolidCube(character.width);

	//foot
	glTranslatef(0.0f, -0.05f, 0.0f);
	glScalef(1.0f, 0.4f, 0.0f);
	glutSolidCube(character.width);
	glPopMatrix();

	//몸
	glPushMatrix();
	glTranslatef(character.x, character.y + 0.2f, 0.0f);
	glColor3f(1, 0, 1);
	glScalef(0.5f, 1.0f, 0.0f);
	glutSolidCube(character.width);
	glPopMatrix();

	//골판
	glPushMatrix();
	glTranslatef(character.x, character.y + 0.15f, 0.0f);
	glColor3f(1, 0, 1);
	glScalef(0.45f, 0.5f, 0.0f);
	glutSolidCube(character.width);
	glPopMatrix();
	
	//오른팔
	//upper
	glPushMatrix();
	glTranslatef(character.x, character.y + 0.2f, 0.0f);
	glColor3f(0.9, 0, 1);
	glRotatef(-upperarmAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.001f, 0.0f);
	glScalef(0.3f, 0.6f, 0.0f);
	glutSolidCube(character.width);

	//lower
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(r_lowerarmAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	glScalef(1.0f, 1.0f, 0.0f);
	glutSolidCube(character.width);

	//hand
	glTranslatef(0.0f, -0.08f, 0.0f);
	glScalef(0.5f, 0.5f, 0.0f);
	glutSolidCube(character.width);
	glPopMatrix();
	
	//오른발
	//upper
	glPushMatrix();
	glTranslatef(character.x, character.y + 0.15f, 0.0f);
	glColor3f(0.9, 0, 1);
	glRotatef(-upperlegAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.05f, 0.0f);
	glScalef(0.3f, 0.7f, 0.0f);
	glutSolidCube(character.width);

	//lower
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(r_lowerlegAngles[frame], 0.0f, 0.0f, 1.0f);
	glTranslatef(0.0f, -0.1f, 0.0f);
	glScalef(0.8f, 1.0f, 0.0f);
	glutSolidCube(character.width);

	//foot
	glTranslatef(0.0f, -0.05f, 0.0f);
	glScalef(1.0f, 0.4f, 0.0f);
	glutSolidCube(character.width);
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
void jump()
{
	if (!character.isJumping && !character.isDoubleJumping)
	{
		character.isJumping = true;
		character.isDoubleJumping = false;
	}
	else if (character.isJumping)
	{
		character.isJumping = false;
		character.isDoubleJumping = true;

	}
	JUMP_SPEED = 0.03f;
}



void checkStarCollision(Character character, Star& star)
{
	if (abs(character.x - viewportX - star.x) < character.width + 0.04f &&
		abs(character.y - GROUND_Y - star.height - 0.1f) < character.height + 0.1f) {
		if (star.collected == false) {
			score++;
			star.collected = true;
		}
		std::cout << "STAR1!!" << std::endl;
	}

	if (abs(character.x - viewportX - star.x) < character.width + 0.1f &&
		abs(character.y - GROUND_Y - star.height - 0.1f) < 0.04f) {
		if (star.collected == false) {
			score++;
			star.collected = true;
		}
		std::cout << "STAR2!!" << std::endl;
	}

	if (abs(character.x - viewportX - star.x) < 0.04f &&
		abs(character.y - GROUND_Y - star.height - 0.1f) < character.height + 0.1f) {
		if (star.collected == false) {
			score++;
			star.collected = true;
		}
		std::cout << "STAR3!!" << std::endl;
	}

	if (abs(character.x - viewportX - star.x) < 0.04f &&
		abs(character.y - GROUND_Y - star.height - 0.1f) < 0.1f) {
		if (star.collected == false) {
			score++;
			star.collected = true;
		}
		std::cout << "STAR4!!" << std::endl;
	}


}

void checkGameOver()
{
	// 게임 종료 조건 확인
	/*캐릭터가 구멍에 빠졌거나 fireball에 접촉하면*/
	if (abs(character.x - viewportX - fb.windowX - 0.2f) < character.width + fb.radius &&
		abs(character.y - GROUND_Y + fb.height) < character.height + fb.radius) {
		//std::cout << "out1" << std::endl;
		gameOver = true;
	}

	if (abs(character.x - viewportX - fb.windowX - 0.2f) < character.width + fb.radius &&
		abs(character.y - GROUND_Y + fb.height) < fb.radius) {
		//std::cout << "out2" << std::endl;
		gameOver = true;
	}

	if (abs(character.x - viewportX - fb.windowX - 0.2f) < fb.radius &&
		abs(character.y - GROUND_Y + fb.height) < character.height + fb.radius) {
		//std::cout << "out3" << std::endl;
		gameOver = true;
	}

	if (abs(character.x - viewportX - fb.windowX - 0.2f) < fb.radius &&
		abs(character.y - GROUND_Y + fb.height) < fb.radius) {
		//std::cout << "out4" << std::endl;
		gameOver = true;
	}

	if (character.x > viewportX && character.x + 0.1f < viewportX + HOLE_WIDTH && character.y < GROUND_Y + 0.01f)
	{
		isFalling = true;
	}
	if (isFalling && character.y < -0.8f)
		gameOver = true;

}



// 게임 로직
void update(int value)
{

	viewportX += MAP_VELOCITY;
	if (viewportX < -2.0f)
	{
		star.collected = false;
		srand(time(NULL)); // 랜덤 요소 추가
		viewportX = 1.0f + ((double)rand() / RAND_MAX);

		enabledStarList.clear();

		srand(time(NULL));
		int count = 0;
		int random = rand() % starList.size();

		for (std::list<Star>::iterator it = starList.begin(); it != starList.end(); it++) {

			if ((random - count <= 1) && (random - count >= -1))
			{
				//std::cout << "추가" << std::endl;
				(*it).collected = false;
				enabledStarList.push_back(*it);
			}
			count++;
		}

	}
	fb.updatePosition(viewportX);
	if (character.isJumping || character.isDoubleJumping || isFalling)
	{
		character.y += JUMP_SPEED;
		JUMP_SPEED += GRAVITY;


	}

	if (character.isJumping && character.y > GROUND_Y && isFalling)
	{
		isFalling = false;
	}
	if (character.y < GROUND_Y && !isFalling)
	{
		character.isDoubleJumping = false;
		character.isJumping = false;
		character.y = GROUND_Y;
	}


	// 맵의 다른 요소들 업데이트 (fireball, stars 등)


	/*충돌 체크*/
	for (std::list<Star>::iterator it = enabledStarList.begin(); it != enabledStarList.end(); it++) {
		drawStar(*it, viewportX);
		checkStarCollision(character, *it);
	}
	//checkGameOver();

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
		if (!character.isDoubleJumping)
			jump();
		break;
	}
	//glutPostRedisplay();
}


void drawStars()
{

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
	drawTerrain();
	drawScore();

	drawHole(viewportX);
	// 캐릭터 그리기

	static int frame = 0;
	drawCharacter(character, frame);
	frame = (frame + 4) % 4;

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
	glutDisplayFunc(display);

	glutTimerFunc(0, update, 0);
	glewInit();

	glutMainLoop();
	return 0;
}