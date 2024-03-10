/*Implement asynchronous log buffer*/

#ifndef __M_BUF_H__
#define __M_BUF_H__

#include <iostream>
#include <vector>
#include <cassert>

namespace Logs
{
    #define DEFAULT_BUFFER_SIZE (4 * 1024 * 1024)//4MB
    //The threshold used for expansion. If it doesn't exceed, it will double. If it exceeds, it will grow linearly
    #define THRESHOLD_BUFFER_SIZE (10 * 1024 * 1024)
    #define INCREMENT_BUFFER_SIZE (1 * 1024 * 1024)//linear growth

    class Buffer
    {
    public:
        Buffer() :_buffer(DEFAULT_BUFFER_SIZE), _reader_idx(0), _writer_idx(0) {}

        //Determine whether the buffer is empty
        bool empty() {return _reader_idx == _writer_idx; }

        //Returns the starting address of readable data
        const char* begin() {return &_buffer[_reader_idx]; }

        //Returns the length of readable data
        size_t readAbleSize()
        {
            //The current buffer is a double buffer, which will be exchanged after processing and will not be used in loop
            return _writer_idx - _reader_idx;
        }

        //Returns the length of writable data
        size_t writeAbleSize()
        {
            //This interface is only provided for fixed size buffer
            return _buffer.size() - _writer_idx;
        }

        //Exchange data between two buffers
        void swap(Buffer& buf)
        {
            _buffer.swap(buf._buffer);
            std::swap(_reader_idx, buf._reader_idx);
            std::swap(_writer_idx, buf._writer_idx);
        }

        //Write data to buffer
        void push(const char* data, size_t len)
        {
            //assert(len <= writeAbleSize());//Block

            //1、Not enough space, then expand
            ensureEnoughSize(len);//Dynamic expansion
            
            //2、Copy data into buffer
            std::copy(data, data + len, &_buffer[_writer_idx]);
            //3、Offset backward from current writing position
            //The first step ensures that +len will not exceed the total buffer size
            _writer_idx += len;
        } 

        //Offset the read and write pointers backward
        void pop(size_t len)//Offset moveReader backward after reading data
        {
            assert(len <= readAbleSize());
            _reader_idx += len;
        }

        //When there is no data to read, reset the read and write position and initialize the buffer
        void reset()
        {
            _writer_idx = 0;//All space in the buffer is free
            _reader_idx = 0;//Equality with _writer_idx means no data is readable
        }
    private:
        //Expansd buffer
        void ensureEnoughSize(size_t len)
        {
            if(len <= writeAbleSize()) return ;
            size_t new_size = 0;
            if (_buffer.size() < THRESHOLD_BUFFER_SIZE)
                new_size = _buffer.size() * 2 + len;
            else new_size = _buffer.size() + INCREMENT_BUFFER_SIZE + len;
            _buffer.resize(new_size);
        }
    private:
        std::vector<char> _buffer;
        size_t _reader_idx;
        size_t _writer_idx;
    };
}

#endif