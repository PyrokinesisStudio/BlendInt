/*
 * This file is part of BIL (Blender Interface Library).
 *
 * BIL (Blender Interface Library) is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser
 * General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * BIL (Blender Interface Library) is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with BIL.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * Contributor(s): Freeman Zhang <zhanggyb@gmail.com>
 */

#include <GL/glew.h>

#include <stdio.h>
#include <stdlib.h>

#include <BIL/Shader.hpp>

namespace BIL {

	Shader::Shader ()
	: shader_(0)
	{
	}

	Shader::Shader (const std::string& filename, GLenum type)
	: shader_(0)
	{
		char* buf = read (filename.c_str());

		if(buf) {
			shader_ = compile (buf, type);
			free(buf);
		}
	}

	Shader::Shader(const char* buf, GLenum type)
	: shader_(0)
	{
		if(buf) {
			shader_ = compile (buf, type);
		}
	}

	Shader::~Shader ()
	{
		if(glIsShader(shader_)) {
			glDeleteShader (shader_);
			shader_ = 0;
		}
	}

	GLenum Shader::type () const
	{
		GLint type;
		glGetShaderiv (shader_, GL_SHADER_TYPE, &type);

		return type;
	}

	bool Shader::isDeleted() const
	{
		GLint status;
		glGetShaderiv (shader_, GL_DELETE_STATUS, &status);

		return status == GL_TRUE? true : false;
	}

	void Shader::load (const std::string& filename, GLenum type)
	{
		if(glIsShader(shader_)) {
			glDeleteShader (shader_);
			shader_ = 0;
		}

		char* buf = read (filename.c_str());

		if(buf) {
			shader_ = compile (buf, type);
			free (buf);
		}
	}

	void Shader::load (const char* buf, GLenum type)
	{
		if(glIsShader(shader_)) {
			glDeleteShader (shader_);
			shader_ = 0;
		}

		if(buf) {
			shader_ = compile (buf, type);
		}
	}

	char* Shader::read (const char* filename)
	{
		FILE * file;
		char * buffer;
		size_t size;

		file = fopen(filename, "rb");
		if (!file) {
			fprintf( stderr, "Unable to open file \"%s\".\n", filename);
			return NULL;
		}
		fseek(file, 0, SEEK_END);
		size = ftell(file);
		fseek(file, 0, SEEK_SET);
		buffer = (char *) malloc((size + 1) * sizeof(char *));
		fread(buffer, sizeof(char), size, file);
		buffer[size] = '\0';
		fclose(file);
		return buffer;
	}

	GLuint Shader::compile (const char* source, const GLenum type)
	{
		GLint compile_status;
		GLuint handle = glCreateShader(type);
		glShaderSource(handle, 1, &source, 0);
		glCompileShader(handle);

		glGetShaderiv(handle, GL_COMPILE_STATUS, &compile_status);
		if (compile_status == GL_FALSE) {
			GLchar messages[256];
			glGetShaderInfoLog(handle, sizeof(messages), 0, &messages[0]);
			fprintf( stderr, "%s\n", messages);
			exit( EXIT_FAILURE);
		}
		return handle;
	}

}
