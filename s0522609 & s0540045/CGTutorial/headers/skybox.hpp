#ifndef SKYBOX_HPP
#define SKYBOX_HPP

class Skybox
{
public:
	Skybox();
	~Skybox();
	
	void zeichneSkybox(GLuint programID, glm::mat4 Projection, glm::mat4 View, std::vector<glm::mat4> Models);
	void zeichneSkybox(GLuint programID, glm::mat4 Projection, glm::mat4 View, glm::mat4 Models);
private:
	GLfloat vertex_buffer_data[18];
	GLfloat uv_buffer_data[12];
	std::vector<GLuint> Textures;
	GLuint sky_vertexbuffer;
	GLuint sky_uvbuffer;
	GLuint Texture;

};

#endif