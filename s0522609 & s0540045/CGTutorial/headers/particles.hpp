#ifndef PARTICLES_HPP
#define PARTICLES_HPP

struct Particle{
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;
	float life; // Remaining life of the particle. if <0 : dead and unused.
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};

class Particles
{
public:
	Particles(GLuint programID);
	~Particles();
	void draw_particles(glm::mat4 View, glm::mat4 Projection, float delta);
	
	// Finds a Particle in ParticlesContainer which isn't used yet.
	// (i.e. life < 0);
	int FindUnusedParticle();

	void SortParticles();

private:
	
	const int MaxParticles = 10;
	Particle ParticlesContainer[10];
	int LastUsedParticle = 0;

	GLuint VertexArrayID_particles;
	GLuint programID_particles;
	GLuint old_programID;
	GLuint CameraRight_worldspace_ID;
	GLuint CameraUp_worldspace_ID;
	GLuint ViewProjMatrixID;
	GLuint TextureID_particles;
	GLfloat* g_particule_position_size_data;
	GLubyte* g_particule_color_data;
	GLuint Texture_particles;
	GLfloat g_vertex_buffer_data[12];
	GLuint billboard_vertex_buffer;
	GLuint particles_position_buffer;
	GLuint particles_color_buffer;


};

#endif