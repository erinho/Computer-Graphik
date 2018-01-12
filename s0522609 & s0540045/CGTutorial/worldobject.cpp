#include <vector>
#include <iostream>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "headers/shader.hpp"
#include "headers/objloader.hpp"
#include "headers/texture.hpp"
#include "headers/worldobject.hpp"

World_Object::World_Object(const char *model_path, const char *texture_path)
{
	objectloaderhelper(model_path);
	Texture = loadBMP_custom(texture_path);
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
	int i = vertices.size();

	//kleinste und größte Eckpunkte des Models ermitteln
	min_x = vertices.front().x;
	max_x = min_x;
	min_y = vertices.front().y;
	max_y = min_y;
	min_z = vertices.front().z;
	max_z = min_z;

	for (int j = 0; j < i; j++) {
		glm::vec3 vectr = vertices[j];

		if (vertices[j].x < min_x) min_x = vertices[j].x;
		if (vertices[j].x > max_x) max_x = vertices[j].x;
		if (vertices[j].y < min_y) min_y = vertices[j].y;
		if (vertices[j].y > max_y) max_y = vertices[j].y;
		if (vertices[j].z < min_z) min_z = vertices[j].z;
		if (vertices[j].z > max_z) max_z = vertices[j].z;
	}
	//aus den Eckpunkten die Mitte bestimmen und den Abstand von der Mitte zu Eckpunkten
	size = glm::vec3(max_x - min_x, max_y - min_y, max_z - min_z);
	center = glm::vec3((min_x + max_x) / 2, (min_y + max_y) / 2, (min_z + max_z) / 2);
	transform = glm::translate(glm::mat4(1), center)* glm::scale(glm::mat4(1), size);
	this->adjustBoundingBox();
}

World_Object::~World_Object()
{
	glDeleteBuffers(1, &uvbuffer);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);
	
}

void World_Object::zeichneObjekt(GLuint programID, glm::mat4 Projection, glm::mat4 View) {

	glm::mat4 MVPold = Projection * View * self_mat;

	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &self_mat[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "V"), 1, GL_FALSE, &View[0][0]);

	glm::mat4 MVP_new = Projection * View * self_mat;
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVP_new[0][0]);

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Texture);

	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);

	glBindVertexArray(VertexArrayID);

	// Das folgende koennte auch aussehalb der Schleife stehen, wenn wir mit glBindVertexArray arbeiten
	glEnableVertexAttribArray(0); // siehe layout im vertex shader 


	//--v-- in Uebung 2 eigefuegt --v--
	glEnableVertexAttribArray(2); // siehe layout im vertex shader
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); // siehe layout im vertex shader (1ster Parameter - 2)?



	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0); 	// siehe layout im vertex shader 
	glDrawArrays(GL_TRIANGLES, 0, vertices.size()); //oder GL_LINES

	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &self_mat[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVPold[0][0]);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

}

void World_Object::objectloaderhelper(const char * path) {
	bool res = loadOBJ(path, vertices, uvs, normals);

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
}

void World_Object::translate_self(mat4 where, vec3 vector) {
	self_mat = glm::translate(where, vector);
}

void World_Object::rotate_self(mat4 where, float degrees, vec3 vector) {
	self_mat = glm::rotate(where, degrees, vector);
}

void World_Object::scale_self(mat4 where, vec3 vector) {
	self_mat = glm::scale(where, vector);
}

mat4 World_Object::get_self() {
	return self_mat;
}

mat4 World_Object::get_m() {
	this->adjustBoundingBox();
	return m;
}

void World_Object::adjustBoundingBox() {

	//Transformation auf des Objekts auf die Matrix der BoundingBox anwenden
	m = self_mat * transform;
	//vom Mittelpunkt der Matrix wieder auf die Eckpunkte schließen
	max_x = m[3].x + size.x / 2;
	min_x = m[3].x - size.x / 2;
	max_y = m[3].y + size.y / 2;
	min_y = m[3].y - size.y / 2;
	max_z = m[3].z + size.z / 2;
	min_z = m[3].z - size.z / 2;
}

bool World_Object::test_collision(float extern_x, float extern_y, float extern_z, vec3 extern_size) {

	this->adjustBoundingBox();
	//Punkt liegt in der BoundingBox -> Kollision
	if ((extern_x < max_x) && (extern_x > min_x)) {
		if ((extern_y < max_y) && (extern_y > min_y)) {
			if ((extern_z < max_z) && (extern_z > min_z)) {
				return true;
			}
		}
	}
	return false;
}

bool World_Object::x_collision(float extern_x,vec3 extern_size){
	this->adjustBoundingBox();
	if ((extern_x < max_x) && (extern_x > min_x)){
		return true;
	}
	return false;
}

bool World_Object::y_collision(float extern_y,vec3 extern_size){
	this->adjustBoundingBox();
	if ((extern_y < max_y) && (extern_y > min_y)){
		return true;
	}
	return false;
}

bool World_Object::z_collision(float extern_z,vec3 extern_size){
	this->adjustBoundingBox();
	if ((extern_z < max_z) && (extern_z > min_z)){
		return true;
	}
	return false;
}