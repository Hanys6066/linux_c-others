#include <GL/glew.h>
#include <GL/glut.h>
#include "graph_lib/graph.h"

#define WINDOW0_HEIGHT 600
#define WINDOW0_WIDTH 800
#define WINDOW0_R 0.48
#define WINDOW0_G 0.48
#define WINDOW0_B 0.48


/**/
int window;
struct g_figure f1;
struct shPrg shader = {
    .Relvs = "../shaders/GLSL_shaders/full_rectangle_flat.vert",
    .Relfs = "../shaders/GLSL_shaders/full_rectangle_flat.frag",
    .Relgs = (char *)NULL,
    .Reles = (char *)NULL,
    .Relcs = (char *)NULL
};

static void clean(){
    figureDelete(&f1);
}


static void onKeyboard(unsigned char key, int x, int y)
{
    if(key==27){
        glutDestroyWindow(window);
        clean();
        exit(EXIT_SUCCESS);
    }
}

static void onMouse(int mouse_button, int state, int x, int y)
{

}

static void onDisplay(void){
    int currentWindow = glutGetWindow();
    glClear(GL_COLOR_BUFFER_BIT);

    if(currentWindow == window){
        glClearColor(WINDOW0_R, WINDOW0_G, WINDOW0_B,0.0);
    }
    drawFigure(&f1);
    glDrawBuffer(GL_FRONT);
    glutSwapBuffers();
    glFlush();
}

static int createWindow(char *title, uint16_t w, uint16_t h, uint16_t L_w, uint16_t L_h)
{
  int window;
  window = glutCreateWindow(title);
  glutReshapeWindow(w,h);
  glutInitWindowPosition(L_w, L_h);
  glutPositionWindow(L_w,L_h);
  return window;
}

static void Reshape(int w, int h)
{
    glViewport(0, 0,(GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, w, 0.0f, h, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);

    rescaleFigure(&f1);
    glutPostRedisplay();
}

static void gl_init(void)
{
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE | GLUT_ALPHA);
    glutDisplayFunc(onDisplay);
    glutMouseFunc(onMouse);
    glutKeyboardFunc(onKeyboard);
    glutReshapeFunc(Reshape);
    glShadeModel(GL_SMOOTH);
    glewInit();
    useShader(&shader);
}

void timer(int value){
    glutTimerFunc(1000, timer, 0);
}


int main(int argc, char **argv){
    glutInit(&argc, argv);
    int line, plot1;
    float xData[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
    float yData[5] = {1.2f, -1.0f, 5.0f, 4.2f, 6.7f};
    float xData2[6] = {6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f};
    float yData2[6] = {1.0f, 1.2f, 1.8f, 1.8f, 5.0f, -1.2f};
    window = createWindow("Example", WINDOW0_WIDTH, WINDOW0_HEIGHT, 0, 0);
    gl_init();

    f1 = create_figure(0.1f, 0.1f, 0.8f, 0.8f);
    plot1 = add_plot(&f1, 0.1f, 0.1f, 0.8f, 0.8f);

    /*diag*/
    //add_axis(&(f1.plots[0]), 0.05f, 0.05f, 0.95f, 0.95f);

    /**/
    line = add_xyline(&(f1.plots[0]), xData, yData, 5);
    //setLineData(&(f1.plots[plot1]), line, xData2, yData2, 6);
    lineAddData(&(f1.plots[plot1]), line, xData2, yData2, 6);
    glutTimerFunc(1000, timer, 0);
    glutMainLoop();
    clean();
    return 0;
}
