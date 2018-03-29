#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

namespace ogl
{

class IndexBuffer
{
public:
    IndexBuffer(const void *, size_t);
	~IndexBuffer();

	unsigned GetID() const { return _buffer_id; }
protected:
    unsigned _buffer_id;
};

} // namespace ogl

#endif