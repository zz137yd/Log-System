/*实现异步日志缓冲区*/

#ifndef __M_BUF_H__
#define __M_BUF_H__

#include <iostream>
#include <vector>
#include <cassert>

namespace Logs
{
    #define DEFAULT_BUFFER_SIZE (4 * 1024 * 1024)//4MB
    #define THRESHOLD_BUFFER_SIZE (10 * 1024 * 1024)//扩容用的阈值, 不超过就翻倍增长, 超过就线性增长
    #define INCREMENT_BUFFER_SIZE (1 * 1024 * 1024)//线性增长

    class Buffer
    {
    public:
        Buffer() :_buffer(DEFAULT_BUFFER_SIZE), _reader_idx(0), _writer_idx(0) {}

        //判断缓冲区是否为空
        bool empty() {return _reader_idx == _writer_idx; }

        //返回可读数据的起始地址
        const char* begin() {return &_buffer[_reader_idx]; }

        //返回可读数据的长度
        size_t readAbleSize()
        {
            //当前缓冲区是双缓冲区, 处理完就交换, 不做循环使用
            return _writer_idx - _reader_idx;
        }

        //返回可写数据的长度
        size_t writeAbleSize()
        {
            //这个接口仅为固定大小缓冲区提供
            return _buffer.size() - _writer_idx;
        }

        //对两个缓冲区进行数据交换
        void swap(Buffer& buf)
        {
            _buffer.swap(buf._buffer);
            std::swap(_reader_idx, buf._reader_idx);
            std::swap(_writer_idx, buf._writer_idx);
        }

        //向缓冲区写入数据
        void push(const char* data, size_t len)
        {
            //assert(len <= writeAbleSize());//阻塞

            //1、空间不够就扩容
            ensureEnoughSize(len);//动态扩容
            
            //2、将数据拷贝进缓冲区
            std::copy(data, data + len, &_buffer[_writer_idx]);
            //3、当前写入位置向后偏移
            //第一步就保证了+len不会超过缓冲区总大小
            _writer_idx += len;
        } 

        //对读写指针进行向后偏移操作
        void pop(size_t len)//读取数据后向后偏移moveReader
        {
            assert(len <= readAbleSize());
            _reader_idx += len;
        }

        //没有数据可读时重置读写位置, 初始化缓冲区
        void reset()
        {
            _writer_idx = 0;//缓冲区所有空间都是空闲的
            _reader_idx = 0;//与_writer_idx相等说明没有数据可读
        }
    private:
        //扩容缓冲区
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