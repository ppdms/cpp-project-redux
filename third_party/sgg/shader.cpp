#include <sgg/shader.h>
#ifndef __EMSCRIPTEN__
#include <GL/glew.h>
#endif
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

#ifdef __EMSCRIPTEN__
const char* __DefaultVertexShader = R"glsl(#version 300 es
    in vec3 position;
	void main()
    {
        gl_Position = vec4(position, 1.0);
    }
)glsl";


const char* __DefaultFragmentShader = R"glsl(#version 300 es
    precision mediump float;
    out vec4 fragcolor;
    void main()
    {
        fragcolor = vec4(0.0,1.0,0.0,1.0);
    }
)glsl";
#else
const char* __DefaultVertexShader = R"glsl(#version 120
    in vec3 position;
	void main()
    {
        gl_Position = vec4(position, 1.0);
    }
)glsl";


const char* __DefaultFragmentShader = R"glsl(#version 120
    out vec4 fragcolor;
    void main()
    {
        fragcolor = vec4(0.0,1.0,0.0,1.0);
    }
)glsl";
#endif

void Shader::printLog(unsigned int object)
{
	GLint log_length = 0;
	if (glIsShader(object))
		glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else if (glIsProgram(object))
		glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
	else
	{
		printf("printlog: Not a shader or a program\n");
		return;
	}
	if (log_length == 0)
		return;

	char* log = new char[log_length];

	if (glIsShader(object))
		glGetShaderInfoLog(object, log_length, NULL, log);
	else if (glIsProgram(object))
		glGetProgramInfoLog(object, log_length, NULL, log);

	printf("%s", log);
	delete[] log;
}
Shader::Shader(const char * vertex, const char * fragment, const char * geometry)
{

	if (!vertex || !fragment)
		return;

	vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vertex, NULL);

	fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, 1, &fragment, NULL);

#ifdef __EMSCRIPTEN__
	// WebGL2 / GLSL ES 3.0 has no geometry shaders; SGG never passes one.
	(void)geometry;
#else
	if (geometry)
	{
		gshader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(gshader, 1, &geometry, NULL);
	}
#endif

	GLint status;

	glCompileShader(vshader);
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		printLog(vshader);
		return;
	}
	glCompileShader(fshader);
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		printLog(fshader);
		return;
	}

#ifndef __EMSCRIPTEN__
	if (geometry)
	{
		glCompileShader(gshader);
		glGetShaderiv(gshader, GL_COMPILE_STATUS, &status);
		if (!status)
		{
			printLog(gshader);
			return;
		}
	}
#endif

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);
#ifndef __EMSCRIPTEN__
	if (geometry)
		glAttachShader(program, gshader);
#endif

	glLinkProgram(program);

	assert(glGetError() == GL_NO_ERROR);
//	GLint status;

	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (!status)
	{
		printLog(program);
		return;
	}

	glValidateProgram(program);

	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	if (!status)
	{
		printLog(program);
		return;
	}

}

Uniform & Shader::operator[](const char * name)
{
	
	auto iter = uniforms.find(name);
	if (iter == uniforms.end())
	{
		Uniform uni = glGetUniformLocation(program, name);
		
		uniforms[name] = uni;
		return uniforms[name];
	}
	else
	{
		return iter->second;
	}	
	
}

bool Shader::use(bool use)
{
	if (!ready)
		return false;

	if (use)
		glUseProgram(program);
	else
		glUseProgram(0);

	return true;
}

std::string Shader::loadShaderText(char * file)
{
	return std::string();
}

bool Shader::init()
{
	
	
	ready = true;
	return true;
}

void Shader::setFragmentLocation(char * name, unsigned int location)
{
#ifdef __EMSCRIPTEN__
	// No-op: GLSL ES 3.0 fragment outputs bind via layout qualifiers.
	(void)name;
	(void)location;
#else
	glBindFragDataLocation(program, location, name);
#endif
}

unsigned int Shader::getAttributeLocation(const char * attrib)
{
	return glGetAttribLocation(program, attrib);
}

Shader::~Shader()
{
	if (!program)
		return; // Compilation failed; nothing to detach.
	if (vshader > 0)
	{
		glDetachShader(program, vshader);
		glDeleteShader(vshader);
	}
	if (fshader > 0)
	{
		glDetachShader(program, fshader);
		glDeleteShader(fshader);
	}
	if (gshader > 0)
	{
		glDetachShader(program, gshader);
		glDeleteShader(gshader);
	}
	
}

Uniform::Uniform(const Uniform & right)
{
	id = right.id;
}

Uniform & Uniform::operator=(const Uniform & right)
{
	id = right.id;
	return *this;
}

Uniform & Uniform::operator=(int i)
{
	glUniform1i(this->id, i);
	return *this;
}

Uniform & Uniform::operator=(float f)
{
	glUniform1f(this->id, f);
	return *this;
}

Uniform & Uniform::operator=(unsigned int i)
{
	glUniform1ui(this->id, i);
	return *this;
}

Uniform & Uniform::operator=(glm::vec3 v)
{
	glUniform3f(this->id, v.x, v.y, v.z);
	int err = glGetError();
	assert(err == GL_NO_ERROR);

	return *this;
}

Uniform & Uniform::operator=(glm::vec4 v)
{
	glUniform4f(this->id, v.x, v.y, v.z, v.w);
	return *this;
}

Uniform & Uniform::operator=(glm::vec2 v)
{
	glUniform2fv(this->id, 1, &(v[0]));
	return *this;
}

Uniform & Uniform::operator=(glm::ivec3 v)
{
	glUniform3iv(this->id, 1, &(v[0]));
	return *this;
}

Uniform & Uniform::operator=(glm::ivec2 v)
{
	glUniform2iv(this->id, 1, &(v[0]));
	return *this;
}

Uniform & Uniform::operator=(glm::ivec4 v)
{
	glUniform4iv(this->id, 1, &(v[0]));
	return *this;
}

Uniform & Uniform::operator=(glm::mat4 m)
{
	glUniformMatrix4fv(this->id, 1, false, &(m[0][0]));
	return *this;
}

Uniform & Uniform::operator=(glm::mat3 m)
{
	glUniformMatrix3fv(this->id, 1, false, &(m[0][0]));
	return *this;
}
