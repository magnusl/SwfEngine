#ifndef VERTEX_BUFFER_H
#define VERTEX_BUFFER_H

namespace ogl
{

class VertexBuffer
{
public:
    VertexBuffer(const void *, size_t, unsigned a_Usage);
	~VertexBuffer();

	void	 Update(const void *, size_t);
	unsigned GetID() const {return _buffer_id;}

protected:

	VertexBuffer(const VertexBuffer &);
	VertexBuffer & operator=(const VertexBuffer &);

	unsigned _buffer_id;
    unsigned _usage;
};

} // namespace ogl

#endif