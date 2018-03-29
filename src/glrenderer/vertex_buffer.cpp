#include "stdafx.h"
#include "vertex_buffer.h"
#include <GL\glew.h>

namespace ogl
{

/**
 * \brief	Constructor for oglVertexBuffer
 * \param [in] a_VertexData	The vertex data.
 * \param [in] a_DataSize	The size fo the data.
 */
VertexBuffer::VertexBuffer(const void * a_VertexData, size_t a_DataSize, GLenum a_Usage) : _usage(a_Usage)
{
	glGenBuffers(1, &_buffer_id);
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_id);
	glBufferData(GL_ARRAY_BUFFER, a_DataSize, a_VertexData, _usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::Update(const void * a_VertexData, size_t a_DataSize)
{
	glBindBuffer(GL_ARRAY_BUFFER, _buffer_id);
	glBufferData(GL_ARRAY_BUFFER, a_DataSize, a_VertexData, _usage);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

/**
 * \brief	Destructor. Performs the required cleanup.
 */
VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &_buffer_id);
}

} // namespace ogl