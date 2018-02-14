//
//modified by: Kyle Gregory, George Brandlin, Heriberto Jimenez, Kasean Dunham
//date: Febuary 13, 2018
//
//3350 Spring 2018 Homework-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
// .general animation framework
// .animation loop
// .object definition and movement
// .collision detection
// .mouse/keyboard interaction
// .object constructor
// .coding style
// .defined constants
// .use of static variables
// .dynamic memory allocation
// .simple opengl components
// .git
//
//elements we will add to program...
//   .Game constructor
//   .multiple particles
//   .gravity
//   .collision detection
//   .more objects
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

const int MAX_PARTICLES = 5000;
const int MAX_BOXES = 5;
const float GRAVITY = 0.1;

char st[5][15] = {{"Requirements"},
	{"Design"},
	{"Coding"},
	{"Testing"},
	{"Maintenance"}};

//some structures

struct Vec {
	float x, y, z;
};

struct Shape {
	float width, height;
	float radius;
	Vec center;
};

struct Particle {
	Shape s;
	Shape c;
	Vec velocity;
};

class Global {
	public:
		int xres, yres;
		Shape box[MAX_BOXES];
		Shape circle;
		Particle particle[MAX_PARTICLES];
		int n;
		Global() {
			xres = 800;
			yres = 600;
			//define a box shape
			circle.radius = 150;
			circle.center.x = 700;
			circle.center.y = -50;
			for(int i=0; i < MAX_BOXES; i++) {
				box[i].width = 100;
				box[i].height = 10;
				box[i].center.x = 120 + i*65;
				box[i].center.y = 500 - i*60;
			}
			n = 0;
		}
} g;

class X11_wrapper {
	private:
		Display *dpy;
		Window win;
		GLXContext glc;
	public:
		~X11_wrapper() {
			XDestroyWindow(dpy, win);
			XCloseDisplay(dpy);
		}
		X11_wrapper() {
			GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
			int w = g.xres, h = g.yres;
			dpy = XOpenDisplay(NULL);
			if (dpy == NULL) {
				cout << "\n\tcannot connect to X server\n" << endl;
				exit(EXIT_FAILURE);
			}
			Window root = DefaultRootWindow(dpy);
			XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
			if (vi == NULL) {
				cout << "\n\tno appropriate visual found\n" << endl;
				exit(EXIT_FAILURE);
			} 
			Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
			XSetWindowAttributes swa;
			swa.colormap = cmap;
			swa.event_mask =
				ExposureMask | KeyPressMask | KeyReleaseMask |
				ButtonPress | ButtonReleaseMask |
				PointerMotionMask |
				StructureNotifyMask | SubstructureNotifyMask;
			win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
					InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
			set_title();
			glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
			glXMakeCurrent(dpy, win, glc);
		}
		void set_title() {
			//Set the window title bar.
			XMapWindow(dpy, win);
			XStoreName(dpy, win, "3350 Lab1");
		}
		bool getXPending() {
			//See if there are pending events.
			return XPending(dpy);
		}
		XEvent getXNextEvent() {
			//Get a pending event.
			XEvent e;
			XNextEvent(dpy, &e);
			return e;
		}
		void swapBuffers() {
			glXSwapBuffers(dpy, win);
		}
} x11;

//Function prototypes
void init_opengl(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render();



//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
	srand(time(NULL));
	init_opengl();
	//Main animation loop
	int done = 0;
	while (!done) {
		//Process external events.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			check_mouse(&e);
			done = check_keys(&e);
		}
		movement();
		render();
		x11.swapBuffers();
	}
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glEnable(GL_TEXTURE_2D);
	//    initialize_fonts();
}

void makeParticle(int x, int y)
{
	if (g.n >= MAX_PARTICLES)
		return;
	cout << "makeParticle() " << x << " " << y << endl;
	//position of particle
	Particle *p = &g.particle[g.n];
	p->s.center.x = x;
	p->s.center.y = y;
	p->velocity.y = ((float)rand() / (float)RAND_MAX) * 1.0;
	//Allow particles to move left and right by adding ( - 0.5 ).
	p->velocity.x = ((float)rand() / (float)RAND_MAX) * 1.0;
	++g.n;
}

void check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	if (e->type != ButtonRelease &&
			e->type != ButtonPress &&
			e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = g.yres - e->xbutton.y;
			for (int i=0; i<10; i++) 
				makeParticle(e->xbutton.x, y);
			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed
			return;
		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			int y = g.yres - e->xbutton.y;
			for (int i=0; i<10; i++)
				makeParticle(e->xbutton.x, y);
		}
	}
}

int check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	int y = g.yres -e->xkey.y;
	if (e->type == KeyPress) {
		switch (key) {
			case XK_1:
				//Key 1 was pressed
				for(int i=0; i<20; i++) {
					makeParticle(e->xkey.x, y);
				}
				break;
			case XK_a:
				//Key A was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}

void movement()
{
	Particle *p;	

	if (g.n <= 0)
		return;

	for (int i=0; i<g.n; i++) {
		//Particle *p = &g.particle[i];
		p = &g.particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= GRAVITY;


		//check for collision with shapes...
		Shape *s [MAX_BOXES];
		for(int j=0; j < MAX_BOXES; j++) {
			s[j] = &g.box[j];
			if (p->s.center.y < s[j]->center.y + s[j]->height && 
					p->s.center.y > s[j]->center.y - s[j]->height && 
					p->s.center.x > s[j]->center.x - s[j]->width &&
					p->s.center.x < s[j]->center.x + s[j]->width) {
				p->velocity.y = -p->velocity.y;
				p->velocity.y *= 0.5;
			}
		}

		Shape *c;
		c = &g.circle;
		if (p->c.center.y < c->center.y + c->radius && 
				p->c.center.y > c->center.y - c->radius && 
				p->c.center.x > c->center.x - c->radius &&
				p->c.center.x < c->center.x + c->radius) {
			p->velocity.y = -p->velocity.y;
			p->velocity.y *= 0.5;
		}

		//check for off-screen
		if (p->s.center.y < 0.0) {
			cout << "off screen" << endl;
			g.particle[i] = g.particle[--g.n];
		} 
	}
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...

	float w, h;
	int col1 = 90;
	int col2 = 140;
	float col3 = 90;

	//
	// draw boxes
	Shape *s [MAX_BOXES];
	for(int i=0; i < MAX_BOXES; i++) {
		glColor3ub(col1,col2,col3);
		s[i] = &g.box[i];
		glPushMatrix();
		glTranslatef(s[i]->center.x, s[i]->center.y, s[i]->center.z);
		w = s[i]->width;
		h = s[i]->height;
		glBegin(GL_QUADS);
		glVertex2i(-w, -h);
		glVertex2i(-w,  h);
		glVertex2i( w,  h);
		glVertex2i( w, -h);
		glEnd();
		glPopMatrix();

		col1 += 50;
		col2 -= 50;
		col3 *= 0.5;

		Rect r;
		r.bot = s[i]->center.y - 10;
		r.left = s[i]->center.x;
		r.center = s[i]->center.x;
		//	unsigned int color = 0x0ffffff;
		char temp[15];
		for (int j=0; j<15; j++) {
			temp[j] = st[i][j];
		}
		//	ggprint13(&r, 20, color, "%s",temp);
	}


	//
	// draw circle
	Shape *c;
	c = &g.circle;
	int x = c->center.x;
	int y = c->center.y;
	int radius = c->radius;
	glPushMatrix();
	glColor3ub(col1,col2,col3);
	double twicePi = 2.0 *3.142;
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y);
	for(int i=0; i < 100; i++) {
		glVertex2i((x + (radius * cos(i * twicePi / 100))), (y + (radius * sin(i * twicePi / 100))));
	}
	glEnd();


	//
	//Draw the particle here
	for (int i=0; i<g.n; i++) {
		glPushMatrix();
		Vec *c = &g.particle[i].s.center;
		glColor3ub(255,0,0);
		w = 2;
		h = 2;
		glBegin(GL_QUADS);
		glVertex2i(c->x-w, c->y-h);
		glVertex2i(c->x-w, c->y+h);
		glVertex2i(c->x+w, c->y+h);
		glVertex2i(c->x+w, c->y-h);
		glEnd();
		glPopMatrix();
	}

	for(int i=0; i<10; i++) {
		makeParticle(g.box[0].center.x-70 ,g.box[0].center.y + 100 );
	}
}
