#include "stdafx.h"
#include "index_buffer.h"
#include <GL/glew.h>

namespace ogl
{

IndexBuffer::IndexBuffer(const void * indices, size_t size)
{
	glGenBuffers(1, &_buffer_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _buffer_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &_buffer_id);
}

} // namespace ogl