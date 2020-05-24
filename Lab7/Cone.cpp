/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Cone class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cone.h"
#include <math.h>

/**
* Cone's intersection method. The input is a ray.
*/
float Cone::intersect(glm::vec3 p0, glm::vec3 dir) {

	glm::vec3 d = p0 - center;
	float yd = height - p0.y + center.y;
	float tan = (radius / height) * (radius / height);
	float a = (dir.x * dir.x) + (dir.z * dir.z) - (tan * (dir.y * dir.y));
	float b = 2 * (d.x * dir.x + d.z * dir.z + tan * yd * dir.y);
	float c = (d.x * d.x) + (d.z * d.z) - (tan * (yd * yd));
	float discriminant = (b * b) - 4 * (a * c);

	if (fabs(discriminant) < 0.001 || discriminant < 0.0) return -1.0;
	
	float t1 = (-b - sqrt(discriminant)) / (2 * a);
	if (t1 < 0.01) t1 = -1;

	float t2 = (-b + sqrt(discriminant)) / (2 * a);
	if (t2 < 0.01) t2 = -1;
	
	float tClose = fmin(t1, t2);
	float tFar = fmax(t1, t2);

	float y = p0.y + dir.y * tClose;
	if ((y >= center.y) && (y <= center.y + height)) return tClose;
	else {
		y = p0.y + dir.y * tFar;
		if ((y >= center.y) && (y <= center.y + height)) return tFar;
	}
	return -1.0;
}


glm::vec3 Cone::normal(glm::vec3 p)
{
	glm::vec3 d = p - center;
	float r = sqrt(d.x * d.x + d.z * d.z);
	glm::vec3 normal = glm::vec3(d.x, r * (radius / height), d.z);
	normal = glm::normalize(normal);
	return normal;
}
