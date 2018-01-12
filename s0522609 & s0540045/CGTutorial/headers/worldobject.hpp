#ifndef WORLDOBJECT_HPP
#define WORLDOBJECT_HPP

class World_Object
{
public:
	World_Object(const char *model_path, const char *texture_path);
	~World_Object();

	void zeichneObjekt(GLuint programID, glm::mat4 Projection, glm::mat4 View);
	void zeichneRotateObjekt(GLuint programID, glm::mat4 Projection, glm::mat4 View);
	mat4 get_self();
	void translate_self(mat4 where, vec3 vector);
	void rotate_self(mat4 where, float degrees, vec3 vector);
	void scale_self(mat4 where, vec3 vector);
	bool test_collision(float extern_x, float extern_y, float extern_z, vec3 extern_size);
	bool x_collision(float extern_x,vec3 extern_size);
	bool y_collision(float extern_y,vec3 extern_size);
	bool z_collision(float extern_z,vec3 extern_size);
	void adjustBoundingBox();
	mat4 get_m();

private:
	mat4 self_mat;
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	
	void objectloaderhelper(const char * path);
	GLuint VertexArrayID;
	GLuint normalbuffer;

	GLuint vertexbuffer;

	GLuint Texture;
	GLuint uvbuffer;

	float min_x;
	float max_x;
	float min_y;
	float max_y;
	float min_z;
	float max_z;

	glm::vec3 size;
	glm::vec3 center;
	glm::mat4 transform;
	glm::mat4 m;

};

#endif