/*==================================================================================
* COSC 363  Computer Graphics (2020)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf, Lab08.pdf for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Plane.h"
#include "Cylinder.h"
#include "TextureBMP.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>
using namespace std;

TextureBMP texture;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;

vector<SceneObject*> sceneObjects;


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(10, 40, -3);					//Light's position
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found

	//Index of checked plane
	if (ray.index == 4)
	{
		//Checkered pattern
		int stripeWidth = 5;
		int stripeHeight = 5;
		int iz = (ray.hit.z) / stripeWidth;
		int ix = (ray.hit.x) / stripeHeight;
		int k = (iz + ix) % 2; //2 colors
		if (k == 0) color = glm::vec3(0, 1, 0);
		else color = glm::vec3(1, 1, 0.5);
		obj->setColor(color);

		//Add code for texture mapping here
		int x1 = -15;
		int x2 = 5;
		int z1 = -60;
		int z2 = -90;

		float texcoords = (ray.hit.x - x1) / (x2 - x1);
		float texcoordt = (ray.hit.z - z1) / (z2 - z1);
			if (texcoords > 0 && texcoords < 1 && texcoordt > 0 && texcoordt < 1){
				color = texture.getColorAt(texcoords, texcoordt);
				obj->setColor(color);
			}
	}



	color = obj->lighting(lightPos, -ray.dir ,ray.hit);						//Object's colour

	//Shadows
	glm::vec3 lightVec = lightPos - ray.hit;
	Ray shadowRay(ray.hit, lightVec);
	shadowRay.closestPt(sceneObjects);
	if (shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec)) {
		color = 0.2f * obj->getColor(); //0.2 = ambient scale factor
	}


	//Reflection
	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}
	return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for(int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

		    glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

		    Ray ray = Ray(eye, dir);

		    glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value
			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}



//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);

    glClearColor(0, 0, 0, 1);

	Sphere *sphere1 = new Sphere(glm::vec3(-5.0, 0.0, -90.0), 15.0);
	sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
	//sphere1->setReflectivity(true, 0.8);
	sphere1->setTransparency(true);
	sceneObjects.push_back(sphere1);		 //Add sphere to scene objects

	Sphere* sphere2 = new Sphere(glm::vec3(5, 5, -70.0), 4.0);
	sphere2->setColor(glm::vec3(0, 1, 1));   //Set colour to blue
	sphere2->setShininess(5);
	sceneObjects.push_back(sphere2);		 //Add sphere to scene objects

	Sphere* sphere3 = new Sphere(glm::vec3(5, -10, -60.0), 5.0);
	sphere3->setColor(glm::vec3(1, 0, 1));   //Set colour to blue
	sphere3->setSpecularity(false);
	sceneObjects.push_back(sphere3);		 //Add sphere to scene objects

	Sphere* sphere4 = new Sphere(glm::vec3(10.0, 10.0, -60.0), 3.0);
	sphere4->setColor(glm::vec3(1, 0, 1));   //Set colour to blue
	sphere4->setSpecularity(false);
	sceneObjects.push_back(sphere4);		 //Add sphere to scene objects

	Plane* plane = new Plane(glm::vec3(-20., -15, -40), //Point A
		glm::vec3(20., -15, -40), //Point B
		glm::vec3(20., -15, -200), //Point C
		glm::vec3(-20., -15, -200)); //Point D'
	plane->setSpecularity(false);
	sceneObjects.push_back(plane);

	Plane* pyramidBase = new Plane(
		glm::vec3(0., 5, -40), //Front left corner
		glm::vec3(2., 5, -40), //Front right corner
		glm::vec3(1., 5, -42)); //Back middle corner
	pyramidBase->setSpecularity(false);
	
	sceneObjects.push_back(pyramidBase);


	Plane* pyramidLeft = new Plane(
		glm::vec3(0., 5, -40), //Point A
		glm::vec3(1., 6, -41), //Point B
		glm::vec3(1., 5, -42)); //Point D'
	pyramidLeft->setSpecularity(false);
	sceneObjects.push_back(pyramidLeft);

	Plane* pyramidRight = new Plane(
		glm::vec3(2., 5, -40),
		glm::vec3(1., 5, -42),
		glm::vec3(1., 6, -41)
		); //Point D'

	pyramidRight->setSpecularity(false);
	sceneObjects.push_back(pyramidRight);

	Plane* pyramidFront = new Plane(
		glm::vec3(0., 5, -40), //Bottom left
		glm::vec3(2., 5, -40), //Bottom right
		glm::vec3(1., 6, -41)); //Top middle
	pyramidFront->setSpecularity(false);
	sceneObjects.push_back(pyramidFront);

	
	



	texture = TextureBMP("Butterfly.bmp");



}


int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
