/*==================================================================================
* COSC 363  Computer Graphics (2020)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf, Lab08.pdf for details.
*===================================================================================
*/

#define _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "Plane.h"
#include "Cylinder.h"
#include "Cone.h"
#include "TextureBMP.h"
#include "SceneObject.h"
#include "Ray.h"
#include <GL/freeglut.h>


using namespace std;

TextureBMP texture;

const float AIR_INDEX = 1.0;
const float GLASS_INDEX = 1.33;
const float ETA = AIR_INDEX/ GLASS_INDEX;

const float BALL_RADIUS = 3.0;
const glm::vec3 BALL_CENTER = glm::vec3(10.0, 5.0, -60.0);

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
//const int MAX_STEPS = 2;
//const int NUMDIV = 350;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;
const float FOGZ1 = -40;
const float FOGZ2 = -600;

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
	if (ray.index == 2)
	{
		//Checkered pattern
		int stripeWidth = 5;
		int stripeHeight = 5;
		int iz = (ray.hit.z) / stripeWidth;
		int ix = (ray.hit.x) / stripeHeight;
		if (ray.hit.x > 0) {
			ix += 1;
		}
		int k = (iz + ix) % 2; //2 colors
		if (k == 0) color = glm::vec3(0, 1, 0);
		else color = glm::vec3(1, 1, 0.5);
		obj->setColor(color);
	}

	if (ray.index == 1) {
		glm::vec3 centerVector = glm::vec3(ray.hit.x - BALL_CENTER.x, ray.hit.y - BALL_CENTER.y, ray.hit.z - BALL_CENTER.z);
		
		float texcoords = 0.5f + atan2(centerVector.x, centerVector.z) / (2 * M_PI);
		float texcoordt = 0.5f - asin(centerVector.y / BALL_RADIUS) / M_PI;

		color = texture.getColorAt(texcoords, texcoordt);
		obj->setColor(color);
	}



	color = obj->lighting(lightPos, -ray.dir ,ray.hit);						//Object's colour

	//Shadows
	glm::vec3 lightVec = lightPos - ray.hit;
	Ray shadowRay(ray.hit, lightVec);
	shadowRay.closestPt(sceneObjects);
	if (shadowRay.index > -1 && shadowRay.dist < glm::length(lightVec)) {
		if (sceneObjects[shadowRay.index]->isTransparent() || sceneObjects[shadowRay.index]->isRefractive()) {
			color = 0.8f * obj->getColor(); //0.8 = ambient scale factor
		}
		else {

			color = 0.2f * obj->getColor(); //0.2 = ambient scale factor
		}
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

	//Transparency
	if (obj->isTransparent() && step < MAX_STEPS) {
		Ray sphereRay = Ray(ray.hit, ray.dir);
		sphereRay.closestPt(sceneObjects);
		Ray outRay = Ray(sphereRay.hit, ray.dir);
		color = color + obj->getTransparencyCoeff() * trace(outRay, step + 1);
	}

	//Refraction
	if (obj->isRefractive() && step < MAX_STEPS) {
		glm::vec3 normal = obj->normal(ray.hit);
		glm::vec3 g = glm::refract(ray.dir, normal, ETA);
		Ray refrRay(ray.hit, g);
		refrRay.closestPt(sceneObjects);
		glm::vec3 m = obj->normal(refrRay.hit);
		glm::vec3 h = glm::refract(g, -m, 1.0f / ETA);
		Ray outRay(refrRay.hit, h);

		color = color + obj->getRefractionCoeff() * trace(outRay, step + 1);
	}


	//Fog
	glm::vec3 fogColour = glm::vec3(1, 1, 1);
	float t = (ray.hit.z - FOGZ1) / (FOGZ2 - FOGZ1);
	color = (1 - t) * color + t * fogColour;

	return color;
}

//Splits pixel into 4 and draws ray through each quarter
glm::vec3 antiAlias(glm::vec3 eye, float cellWidth, float xp, float yp, bool adaptiveSampling, int step) {
	float oneQuarter = cellWidth * 0.25;
	float threeQuarters = cellWidth * 0.75;

	glm::vec3 bottomLeftColour = trace(Ray(eye, glm::vec3(xp + oneQuarter, yp + oneQuarter, -EDIST)), 1);
	glm::vec3 topLeftColour = trace(Ray(eye, glm::vec3(xp + oneQuarter, yp + threeQuarters, -EDIST)), 1);
	glm::vec3 bottomRightColour = trace(Ray(eye, glm::vec3(xp + threeQuarters, yp + oneQuarter, -EDIST)) , 1);
	glm::vec3 topRightColour = trace(Ray(eye, glm::vec3(xp + threeQuarters, yp + threeQuarters, -EDIST)), 1);
	
	glm::vec3 averageColour = (bottomLeftColour + topLeftColour + bottomRightColour + topRightColour) * glm::vec3(0.25);
	

	if (adaptiveSampling && step < 2) {
		glm::vec3 colours[4] = { bottomLeftColour, topLeftColour, bottomRightColour, topRightColour };
		float xArray[4] = {xp + oneQuarter, xp+oneQuarter, xp + threeQuarters, xp+ threeQuarters};
		float yArray[4] = {yp + oneQuarter, yp+threeQuarters, yp + oneQuarter, yp + threeQuarters};

		for (int i = 0; i < 4; i++) {
			glm::vec3 colourDifference = colours[i] - averageColour;
			if (glm::length(colourDifference) > 0.2) {
				colours[i] = antiAlias(eye, cellWidth / 2, xArray[i] - oneQuarter, yArray[i] - oneQuarter, true, step + 1);
			}
		}
		averageColour = (colours[0] + colours[1] + colours[2] + colours[3]) * glm::vec3(0.25);
	}


	return averageColour;
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

			glm::vec3 col = antiAlias(eye, cellX, xp, yp, true, 0);
		    //glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value

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


void createPyramid(glm::vec3 center, float radius, glm::vec3 colour)
{

	glm::vec3 frontLeft = glm::vec3(center.x - radius, center.y, center.z + radius); //Front left corner
	glm::vec3 frontRight = glm::vec3(center.x + radius, center.y, center.z + radius); //Front right corner
	glm::vec3 backMiddle = glm::vec3(center.x, center.y, center.z - radius); //Back middle corner
	glm::vec3 top = glm::vec3(center.x, center.y + radius, center.z);

	Plane* pyramidBase = new Plane(frontLeft, frontRight, backMiddle); 
	Plane* pyramidLeft = new Plane(frontLeft, top, backMiddle);
	Plane* pyramidRight = new Plane(frontRight, top, backMiddle);
	Plane* pyramidFront = new Plane(frontLeft, frontRight, top);

	pyramidBase->setColor(colour);  
	pyramidLeft->setColor(colour);   
	pyramidRight->setColor(colour);   
	pyramidFront->setColor(colour);  

	sceneObjects.push_back(pyramidBase);
	sceneObjects.push_back(pyramidLeft);
	sceneObjects.push_back(pyramidRight);
	sceneObjects.push_back(pyramidFront);
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

	Sphere *sphere1 = new Sphere(glm::vec3(0.0, 0.0, -100.0), 15.0);
	sphere1->setColor(glm::vec3(0, 0, 1));   //Set colour to blue
	sphere1->setReflectivity(true, 0.8);
	sceneObjects.push_back(sphere1);		 //Add sphere to scene objects


	Sphere* texturedSphere = new Sphere(BALL_CENTER, BALL_RADIUS);
	texturedSphere->setColor(glm::vec3(1, 0, 1));   //Set colour to blue
	sceneObjects.push_back(texturedSphere);		 //Add sphere to scene objects

	Plane* plane = new Plane(glm::vec3(-50., -15, -40), //Point A
		glm::vec3(50., -15, -40), //Point B
		glm::vec3(50., -15, -200), //Point C
		glm::vec3(-50., -15, -200)); //Point D'
	plane->setSpecularity(false);
	sceneObjects.push_back(plane);

	Sphere* transparentSphere = new Sphere(glm::vec3(-6.5, -10.0, -60.0), 3.0);
	transparentSphere->setColor(glm::vec3(0.2, 0.2, 0.2));   //Set colour to blue
	transparentSphere->setReflectivity(true, 0.1);
	transparentSphere->setTransparency(true, 0.9);
	sceneObjects.push_back(transparentSphere);		 //Add sphere to scene objects

	Sphere* refractiveSphere = new Sphere(glm::vec3(6.5, -10.0, -60.0), 3.0);
	refractiveSphere->setColor(glm::vec3(0.2, 0.2, 0.2));   //Set colour to blue
	refractiveSphere->setTransparency(true, 0.1);
	refractiveSphere->setReflectivity(true, 0.3);
	refractiveSphere->setRefractivity(true);
	sceneObjects.push_back(refractiveSphere);		 //Add sphere to scene objects

	Cylinder* cylinder = new Cylinder(glm::vec3(-8, -10, -80), 2, 10);
	sceneObjects.push_back(cylinder);

	Cone* cone = new Cone(glm::vec3(0, -10.0, -60), 2, 2);
	sceneObjects.push_back(cone);

	createPyramid(glm::vec3(-8, 0, -80), 3.0, glm::vec3(0,1,1));

	texture = TextureBMP("ball.bmp");
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
