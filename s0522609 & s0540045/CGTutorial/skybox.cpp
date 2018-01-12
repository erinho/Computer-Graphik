#include <vector>

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
#include "headers/skybox.hpp"

Skybox::Skybox() {
	
	Textures.push_back(loadBMP_custom("textures/himmel_front.bmp"));
	Textures.push_back(loadBMP_custom("textures/himmel_boden.bmp"));
	Textures.push_back(loadBMP_custom("textures/himmel_back.bmp"));
	Textures.push_back(loadBMP_custom("textures/himmel_left.bmp"));
	Textures.push_back(loadBMP_custom("textures/himmel_right.bmp"));
	Textures.push_back(loadBMP_custom("textures/himmel_up.bmp"));

	vertex_buffer_data[0] = -1.0f;
	vertex_buffer_data[1] = -1.0f;
	vertex_buffer_data[2] = 0.0f;
	vertex_buffer_data[3] = -1.0f;
	vertex_buffer_data[4] = 1.0f;
	vertex_buffer_data[5] = 0.0f;
	vertex_buffer_data[6] = 1.0f;
	vertex_buffer_data[7] = -1.0f;
	vertex_buffer_data[8] = 0.0f;
	vertex_buffer_data[9] = 1.0f;
	vertex_buffer_data[10] = 1.0f;
	vertex_buffer_data[11] = 0.0f;
	vertex_buffer_data[12] = -1.0f;
	vertex_buffer_data[13] = 1.0f;
	vertex_buffer_data[14] = 0.0f;
	vertex_buffer_data[15] = 1.0f;
	vertex_buffer_data[16] = -1.0f;
	vertex_buffer_data[17] = 0.0f;

	uv_buffer_data[0] = 0.0f;
	uv_buffer_data[1] = 0.0f;
	uv_buffer_data[2] = 0.0f;
	uv_buffer_data[3] = 1.0f;
	uv_buffer_data[4] = 1.0f;
	uv_buffer_data[5] = 0.0f;
	uv_buffer_data[6] = 1.0f;
	uv_buffer_data[7] = 1.0f;
	uv_buffer_data[8] = 0.0f;
	uv_buffer_data[9] = 1.0f;
	uv_buffer_data[10] = 1.0f;
	uv_buffer_data[11] = 0.0f;


	float i;
	glGenBuffers(1, &sky_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sky_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(i), vertex_buffer_data, GL_STATIC_DRAW);


	glGenBuffers(1, &sky_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, sky_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(i), uv_buffer_data, GL_STATIC_DRAW);

}

Skybox::~Skybox() {
	
	glDeleteBuffers(1, &sky_vertexbuffer);
	glDeleteBuffers(1, &sky_uvbuffer);
	for (int i = 0; i < 6; i++) {
		glDeleteTextures(1, &Textures.at(i));
	}
	Textures.clear();

}

void Skybox::zeichneSkybox(GLuint programID, glm::mat4 Projection, glm::mat4 View, std::vector<glm::mat4> Models) {
	
	for (int iter = 0; iter < 6; iter++) {
		glm::mat4 MVPold = Projection * View * Models.at(iter);
		glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Models.at(iter)[0][0]);
		glm::mat4 MVPground = Projection * View * Models.at(iter);
		glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVPground[0][0]);

		// Bind our texture in Texture Unit 0

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Textures.at(iter));
		// Set our "myTextureSampler" sampler to user Texture Unit 0
		glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, sky_vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, sky_uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);

		// Draw the triangle !
		glDepthMask(0);
		glDrawArrays(GL_TRIANGLES, 0, 2 * 3); // 12*3 indices starting at 0 -> 12 triangles
		glDepthMask(1);

		glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Models.at(iter)[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &MVPold[0][0]);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
}


