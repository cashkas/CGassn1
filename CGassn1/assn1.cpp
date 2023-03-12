#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/vec3.hpp>
#include <vector>
#include <ctime>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GROUND_Y = -0.5f;
const float MAP_VELOCITY = -0.01f;
const int FIREBALL_SIZE = 0.01f;
float HOLE_WIDTH = 0.2f;
float viewportX = 1.0f;

class Star {
public:
    Star(float x_, float y_, bool collected_) :
        x(x_), y(y_), collected(false)
    {}
    float x;
    float y;
    int collected;
};

class Fireball {
public:
    Fireball(float x_, float y_, float speed_) :
        x(x_), y(y_), speed(speed_)
    {}

    void move(float dt) {
        x -= speed * dt;
        if (x < -SCREEN_WIDTH) {
            x = INITIAL_X;
        }
    }
private:
    float x;
    float y;
    float speed;
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

Character character = Character(-0.5f, -0.5f, 0.1f, 0.1f);


float characterSpeed = 0.01f; // ĳ������ �̵� �ӵ�

float gravity = -0.001f;
float jumpSpeed = 0.03f;

int score = 0; // ����
bool gameOver = false; // ���� ���� ����


//�ʱ�ȭ
void init()
{
    glLoadIdentity();
}



//draw �Լ�
void drawStar()
{
    glPushMatrix();
    glColor3f(0, 0, 0);

    glPopMatrix();
}
void drawFireball()
{
    glPushMatrix();
    glPopMatrix();

}
void drawTerrain()
{
    glPushMatrix();
    glColor3f(0, 1, 0);
    glBegin(GL_QUADS); // �� �׸���
    glVertex2f(-1.0f, -0.5f);
    glVertex2f(1.0f, -0.5f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
    glPopMatrix();
}
void drawHole(float viewPort)
{
    glPushMatrix();
    glColor3f(0, 0, 0);
    glBegin(GL_QUADS); // ���� �׸���
    glVertex2f(viewPort, -1.0f);
    glVertex2f(viewPort, -0.5f);
    glVertex2f(viewPort + HOLE_WIDTH, -0.5f);
    glVertex2f(viewPort + HOLE_WIDTH, -1.0f);
    glEnd();
    glPopMatrix();
}

void drawCharacter(Character character)
{
    glPushMatrix();

    glTranslatef(character.x, character.y, 0);
    glColor3f(1, 0, 1); // ĳ���� ����

    glBegin(GL_QUADS); // ĳ���� �׸���
    glVertex2f(0, 0);
    glVertex2f(character.width, 0);
    glVertex2f(character.width, character.height);
    glVertex2f(0, character.height);
    glEnd();

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
    jumpSpeed = 0.03f;
}





// ���� ����
void update(int value)
{

    viewportX += MAP_VELOCITY;
    if (viewportX < -1.5f)
    {
        srand(time(NULL)); // ���� ��� �߰�
        viewportX = 1.0f + ((double)rand() / RAND_MAX);
    }
    if (character.isJumping || character.isDoubleJumping)
    {
        character.y += jumpSpeed;
        jumpSpeed += gravity;


    }

    if (character.y < -0.5f) // ���� ������... (���ۿ� ������ ���̽� �߰� �ʿ�)
    {
        character.isDoubleJumping = false;
        character.isJumping = false;
        character.y = -0.5f;
    }

    // ���� �ٸ� ��ҵ� ������Ʈ (fireball, stars ��)

    // ���� ���� ���� Ȯ��

    /*ĳ���Ͱ� ���ۿ� �����ų� fireball�� �����ϸ�*/
    //if(){
    //   gameOver = true;
    //}

    glutTimerFunc(16, update, 0); // ~60fps
    glutPostRedisplay();
}


void keyboard(unsigned char key, int x, int y) // int x, y: ���콺 Ŀ�� ��ġ
{
    switch (key) {
    case 27: // ESC
        exit(0);
        break;
    case 32: // �����̽���
        if (!character.isDoubleJumping)
            jump();
        break;
    }
    glutPostRedisplay();
}




// ������
void display(void) {

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT); //OpenGL�� ������ ���ۿ� �ִ� ��� �ȼ����� ���� ���� clear color�� ������

    // ĳ���� �׸���
    drawCharacter(character);

    // �� �׸��� (terrain, fireball, stars)
    drawFireball();
    drawTerrain();
    drawStar();
    drawHole(viewportX);

    // ���� ����� ���� ǥ��
    if (gameOver) {
        // ���� ǥ��
    }

    // �׸��� �Ϸ� �� ȭ�� ������Ʈ
    glutSwapBuffers();
}




int main(int argc, char** argv) {

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); // ���� ���۸� ���
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT); // ������ ������

    glutCreateWindow("Wind Runner"); // ������ ����
    init();

    glutKeyboardFunc(keyboard);
    glutDisplayFunc(display);

    glutTimerFunc(0, update, 0);
    glewInit();

    glutMainLoop();
    return 0;
}