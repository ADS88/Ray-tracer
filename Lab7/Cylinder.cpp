/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The sphere class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* Cylinders's intersection method.  The input is a ray.
*/
float Cylinder::intersect(glm::vec3 p0, glm::vec3 dir)
{
	glm::vec3 d = p0 - center;
	float a = (dir.x * dir.x) + (dir.z * dir.z);
	float b = 2 * (dir.x * d.x + dir.z * d.z);
	float c = d.x * d.x + d.z * d.z - (radius * radius);

	float discriminant = b * b - 4 * (a * c);
	if (discriminant < 0.0 || (fabs(discriminant)) < 0.001) return -1.0;

	float t1 = (-b - sqrt(discriminant)) / (2 * a);
	if (t1 < 0.01) t1 = -1;

	float t2 = (-b + sqrt(discriminant)) / (2 * a);
	if (t2 < 0.01) t2 = -1;

	float tClose = fmin(t1, t2);
	float tFar = fmax(t1, t2);
	
	float y = p0.y + dir.y * tClose;
	if ((y >= center.y) && (y <= center.y + height)) {
		return tClose;
	}
	else {
		y = p0.y + dir.y * tFar;
		if ((y >= center.y) && (y <= center.y + height)) {
			return tFar;
		}
	}
	return -1.0;
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the cylinder.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 normal = glm::vec3(p.x - center.x, 0, p.z - center.z);
    normal = glm::normalize(normal);
    return normal;
}
