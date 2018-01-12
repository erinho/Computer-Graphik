// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <random>
#include <algorithm>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


using namespace glm;

// Achtung, die OpenGL-Tutorials nutzen glfw 2.7, glfw kommt mit einem veränderten API schon in der Version 3 

#include "headers/shader.hpp"
#include "headers/objloader.hpp"
#include "headers/texture.hpp"
#include "headers/worldobject.hpp"
#include "headers/skybox.hpp"
#include "headers/text2D.hpp"


int pkt = 0;
int pkt_anzahl = 10;
int iceberg = 10;
int spawns = 0;
float rotx = 0.0f;
float roty = 0.0f;
float rotz = 0.0f;
float fov = 45.0f;
float x_mvmt = 0.0f;
float y_mvmt = 0.5f;
float z_mvmt = 0.0f;
float anim_degrees = 0.01f;
float speed = 3.0f;

glm::vec3 anim_vec = glm::vec3(1.0, 0.0, 0.0);
bool win = false;
bool once = true;
bool alive = true;
bool respawn = false;

GLfloat boden_vertex[] = {
	-1.0f, -1.0f, 0.0f,// triangle 1 : begin
	-1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f,// triangle 1 : end
	1.0f, 1.0f, 0.0f,// triangle 2 : begin
	-1.0f, 1.0f, 0.0f,
	1.0f, -1.0f, 0.0f
};
GLfloat boden_uv[] = {
	0.0f, 0.0f,
	0.0f, 10.0f,
	10.0f, 0.0f,
	10.0f, 10.0f,
	0.0f, 10.0f, 
	10.0f, 0.0f
};
GLuint boden_vertexbuffer;
GLuint boden_uvbuffer;
GLuint boden_tex;
glm::vec3 position = glm::vec3(0, 0, -1);
std::default_random_engine generator;
std::uniform_int_distribution<int> pos_distribution(-15, 15);
std::vector<World_Object*> todeskandidaten;

void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;

	case GLFW_KEY_A:
		roty+=0.05;
		if(roty>360.0f || roty<-360.0f) { roty=0.0f; }
		break;

	case GLFW_KEY_D:
		roty -= 0.05;
		if (roty>360.0f || roty<-360.0f) { roty = 0.0f; }
		break;

	default:
		break;
	}
}

void zeichneboden(GLuint programID, glm::mat4 Projection, glm::mat4 View, glm::mat4 Model) {

	glm::mat4 MVPold = Projection * View * Model;

	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Model[0][0]);
	glm::mat4 MVPNew = Projection * View * Model;
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVPNew[0][0]);

	float i;
	glGenBuffers(1, &boden_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, boden_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(i), boden_vertex, GL_STATIC_DRAW);

	glGenBuffers(1, &boden_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, boden_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(i), boden_uv, GL_STATIC_DRAW);


	glActiveTexture(GL_TEXTURE0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glBindTexture(GL_TEXTURE_2D, boden_tex);
	// Set our "myTextureSampler" sampler to user Texture Unit 0
	glUniform1i(glGetUniformLocation(programID, "myTextureSampler"), 0);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, boden_vertexbuffer);
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
	glBindBuffer(GL_ARRAY_BUFFER, boden_uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
		);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 2 * 3); // 12*3 indices starting at 0 -> 12 triangles

	glUniformMatrix4fv(glGetUniformLocation(programID, "M"), 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "MVP"), 1, GL_FALSE, &MVPold[0][0]);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

int main(void)
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(EXIT_FAILURE);
	}
	
	// Fehler werden auf stderr ausgegeben, s. o.
	glfwSetErrorCallback(error_callback);
	
	//--^--Nur Info über Fehler, keine Behandlung--^--

	// Open a window and create its OpenGL context
	// glfwWindowHint vorher aufrufen, um erforderliche Resourcen festzulegen
	GLFWwindow* window = glfwCreateWindow(1028, // Breite
										  720,  // Hoehe
										  "Hovercraft", // Ueberschrift
										  NULL,  // windowed mode
										  NULL); // shared windoe
	
	//--^--Erster Schritt bei Grafikporogrammierung--^--


	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Make the window's context current (wird nicht automatisch gemacht)
    glfwMakeContextCurrent(window);

	//--^--Attribute, etc Speichern--^--

	// Initialize GLEW
	// GLEW ermöglicht Zugriff auf OpenGL-API > 1.1
	glewExperimental = true; // Needed for core profile

	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	//--v--ab hier geht es los--v--

	// Auf Keyboard-Events reagieren
	glfwSetKeyCallback(window, key_callback);

	//--^--automatisch auf Events reagieren--^--

	// RGB + Transparenz
	glClearColor(0.85f, 0.85f, 0.85f, 1.0f);

	//--^--RBG und Alpha(Transparenz)--^--

	// Create and compile our GLSL program from the shaders
	//GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");
	GLuint programID = LoadShaders("shaders/StandardShading.vertexshader", "shaders/StandardShading.fragmentshader");
	
	//--^--VertexShader transformiern die objekte--^--
	//--^--ColorShader stellt die Pixel dar--^--

	// Shader auch benutzen !
	glUseProgram(programID);

	glm::mat4 Model = glm::mat4(1.0f);
	// Jedes Objekt eigenem VA zuordnen, damit mehrere Objekte moeglich sind

	World_Object* templ = new World_Object("models/cube.obj", "textures/iceberg.bmp");
	World_Object* templ_sphere = new World_Object("models/stern.obj", "textures/olympia_yellow.bmp");
	std::vector<World_Object*> Objekte;
	for (int k = 0; k < iceberg; k++) {
		Objekte.push_back(new World_Object(*templ));
		Objekte[k]->translate_self(Model, vec3(pos_distribution(generator),0.005, pos_distribution(generator)));
		Objekte[k]->rotate_self(Objekte[k]->get_self(), 45.0f, vec3(1.0, 1.0, 1.0));
		Objekte[k]->scale_self(Objekte[k]->get_self(), vec3(0.7,0.7,0.7));

	}
	std::vector<World_Object*> Punkte;
	for (int l = 0; l < pkt_anzahl; l++) {
		Punkte.push_back(new World_Object(*templ_sphere));
		Punkte[l]->translate_self(Model, vec3(pos_distribution(generator), 0.25, pos_distribution(generator)));
		Punkte[l]->rotate_self(Punkte[l]->get_self(), 90.0f, vec3(1.0, 0.0, 0.75));
		Punkte[l]->scale_self(Punkte[l]->get_self(), vec3(0.3, 0.3, 0.3));
	}
	World_Object* Pers = new World_Object("models/b/1.obj", "textures/uh60.bmp");
	boden_tex = loadBMP_custom("textures/water.bmp");
	
	glm::mat4 boden_w = Model;
	boden_w = glm::rotate(boden_w, 90.0f, vec3(1.0, 0.0, 0.0));
	boden_w = glm::scale(boden_w, glm::vec3(100.0, 100.0, 100.0));

	Skybox * sky = new Skybox();
	std::vector<glm::mat4> sky_matrizen;

	glm::mat4 ViewHelper = glm::mat4(1.0);
	position += glm::vec3(0.0, 0.05, 0.0);
	int pkt_alt = pkt;

	// Initialize our little text library with the Holstein font
	//von OpenGL Tutorial
	initText2D("textures/holstein.dds");

	double lastTime = glfwGetTime();
	double lastTime_fps = glfwGetTime();
	double anfangs_zeit = lastTime;
	int nbFrames = 0;
	char fps_counter[256];
	sprintf(fps_counter, "init...");
	// Eventloop
	while (!glfwWindowShouldClose(window))
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	

		glEnable(GL_DEPTH_TEST); //--<--Hintere Pixel werden nicht gemalt--<--
		glDepthFunc(GL_LESS);

		//--erster Befehl der Darstellung--
		// Bind our texture in Texture Unit 0

		glUseProgram(programID);
		
		// Projection matrix : Field of View, x:y Display-Ratio, display range : 0.1 unit <-> 100 units
		glm::mat4 Projection = glm::perspective(fov, 16.0f / 9.0f, 0.1f, 100.0f);
		
		glm::vec3 direction(
			cos(rotx) * sin(roty),
			sin(rotx),
			cos(rotx) * cos(roty)
			);

		glm::vec3 right = glm::vec3(
			sin(roty - 3.14f / 2.0f),
			0,
			cos(roty - 3.14f / 2.0f)
			);

		glm::vec3 up = glm::cross(right, direction);
		
		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;

		// * speed * float(delta), damit die Bewegung nicht von der Framerate abhängt
		if (alive) {
			// Move forward
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
				if (position.z < 50.0f) {
					position += direction * speed * float(delta);
					//anim_degrees = 11.0f;
					//anim_vec = glm::vec3(1.0, 0.0, 0.0);
				}
			}

			// Move backward
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
				if (position.z > -50.0f) {
					position -= direction * speed * float(delta);
				}
			}
		}
				
		if (pkt_alt != pkt)
		{
			pkt_alt = pkt;
		}
		if (pkt_anzahl == 0) {
			//Ende, wenn alle Punkte eingesammelt wurden
			alive = false;
			win = true;
		}
		if (alive) {
			//Modelmatrix wird verschoben, damit die Figur um sich selbst rotiert werden kann
			Model = glm::translate(Model, position);
			Pers->translate_self(Model, position);
			Pers->rotate_self(Model, 57.1428f * roty, vec3(0.0, 1.0, 0.0));
			
			Model = glm::translate(Model, -position);
			Pers->translate_self(Pers->get_self(), glm::vec3(0.0, -0.01, -0.5));
			Pers->scale_self(Pers->get_self(), glm::vec3(0.01,0.01,0.01));  //Size from ship
		}
		glm::vec3 test = glm::vec3(0,0.2,0);
		glm::mat4 View = glm::lookAt(
			(position - direction+test),           // Camera is here
			position, // and looks here
			up                  // Head is up
			);

		
		for (int var = 1; var <= pkt_anzahl; var++)
		{
			Punkte[var-1]->zeichneObjekt(programID, Projection, View);

			if (alive)
			{
				if (Punkte[var-1]->test_collision(Pers->get_m()[3].x, Pers->get_m()[3].y, Pers->get_m()[3].z, vec3(1.0, 1.0, 1.0)))
				{
					//Kugeln aus dem Vektor der zu zeichnenden Objekte entfernen, noch nicht löschen, da sonst die anderen Objekte ihre Texturen verlieren
					pkt += 1;
					pkt_anzahl -= 1;
					todeskandidaten.push_back(Punkte[var - 1]);
					Punkte.erase(Punkte.begin() + var-1);
					var = pkt_anzahl + 1;
				}
			}
		}

		zeichneboden(programID, Projection, View, boden_w);
		
		for each (World_Object* var in Objekte)
		{
			var->zeichneObjekt(programID, Projection, View);
			
			if (alive) {
				if (var->test_collision(Pers->get_m()[3].x, Pers->get_m()[3].y, Pers->get_m()[3].z, vec3(1.0, 1.0, 1.0))) {
					if(Pers->get_m()[3].x){
						glm::vec3 testi = glm::vec3(0.5,0,0);
						position = position + testi;
					}
					if(Pers->get_m()[3].z){
						glm::vec3 testin = glm::vec3(0,0,0.5);
						position = position + testin;
					}
					pkt = pkt-1;
				}
			}
		}

		Model = glm::translate(Model, position);
		if (alive) {
			Pers->rotate_self(Pers->get_self(), anim_degrees, anim_vec);
			Pers->zeichneObjekt(programID, Projection, View);
		}
		
		//Skybox
		//immer in der selben Entfernung von der Spielfigur;
		//erreicht durch verschieben der Modelmatrix vorher und nacher

		//vorne
		glm::mat4 front = glm::translate(Model, glm::vec3(0.0, 0.0, 50.0));
		front = glm::rotate(front, 180.0f, glm::vec3(0.0, 1.0, 0.0));
		front = glm::scale(front, glm::vec3(50.0, 50.0, 50.0));
		sky_matrizen.push_back(front);
		
		//unten
		glm::mat4 ground = glm::translate(Model, glm::vec3(0.0, -50.0, 0.0));
		ground = glm::rotate(ground, 90.0f, glm::vec3(1.0, 0.0, 0.0));
		ground = glm::rotate(ground, 90.0f, glm::vec3(0.0, 0.0, 1.0));
		ground = glm::scale(ground, glm::vec3(50.0, 50.0, 50.0));
		sky_matrizen.push_back(ground);
		
		//hinten
		glm::mat4 back = glm::translate(Model, glm::vec3(0.0, 0.0, -50.0));
		back = glm::scale(back, glm::vec3(50.0, 50.0, 50.0));
		sky_matrizen.push_back(back);
		
		//rechts!
		glm::mat4 left = glm::translate(Model, glm::vec3(-50.0, 0.0, 0.0));
		left = glm::rotate(left, 90.0f, glm::vec3(0.0, 1.0, 0.0));
		left = glm::scale(left, glm::vec3(50.0, 50.0, 50.0));
		sky_matrizen.push_back(left);
		
		//links!
		glm::mat4 sky_right = glm::translate(Model, glm::vec3(50.0, 0.0, 0.0));
		sky_right = glm::rotate(sky_right, -90.0f, glm::vec3(0.0, 1.0, 0.0));
		sky_right = glm::scale(sky_right, glm::vec3(50.0, 50.0, 50.0));
		sky_matrizen.push_back(sky_right);
		
		//oben
		glm::mat4 sky_up = glm::translate(Model, glm::vec3(0.0, 50.0, 0.0));
		sky_up = glm::rotate(sky_up, 90.0f, glm::vec3(1.0, 0.0, 0.0));
		sky_up = glm::rotate(sky_up, 90.0f, glm::vec3(0.0, 0.0, 1.0));
		sky_up = glm::scale(sky_up, glm::vec3(50.0, 50.0, 50.0));
		sky_matrizen.push_back(sky_up);

		sky->zeichneSkybox(programID, Projection, View, sky_matrizen);
		sky_matrizen.clear();
		Model = glm::translate(Model, -position);

		//Textausgaben
		//Spielzeit
		char zeit[256];
		sprintf(zeit, "Zeit: %.2f sec", lastTime - anfangs_zeit);
		printText2D(zeit, 1, 520, 30);
		
		//Punktstand
		char pkt_stand[256];
		sprintf(pkt_stand, "Punkte: %d", pkt);
		printText2D(pkt_stand, 1, 485, 30);

		if (win)
		{
			char gewonnen[256];
			sprintf(gewonnen, "Sie haben %d Punkte gesammelt!",pkt);
			printText2D(gewonnen, 125, 320, 20);
		}
		
		//gibt die Durchschnittliche Zeit pro Frame an; von OpenGL Tutorial
		nbFrames++;
		double currentTime_fps = glfwGetTime();

		lastTime = currentTime;

		// Swap buffers
		glfwSwapBuffers(window);

		//--^--letzter Befehl der Darstellung--^--

		// Poll for and process events 
        glfwPollEvents();


		
	} 
	//Objekte und Buffers löschen
	for each (World_Object* var in Objekte)
	{
		var->~World_Object();
	}
	for each (World_Object* var in todeskandidaten)
	{
		var->~World_Object();
	}
	todeskandidaten.clear();
	Objekte.clear();
	sky->~Skybox();
	sky_matrizen.clear();
	cleanupText2D();
	glDeleteBuffers(1, &boden_vertexbuffer);
	glDeleteBuffers(1, &boden_uvbuffer);
	glDeleteTextures(1, &boden_tex);
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}