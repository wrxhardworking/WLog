//
// Created by 28108 on 2022/12/5.
//

#pragma once

#include <cstring>

//前后端统一的buffer
//这是缓冲区用到的buffer
//缓冲区buffer的大小设定
const int LargeBuffer = 4000*1000;   //4MB
const int SmallBuffer = 4000;
//非类型参数的模板类 来指定缓冲buffer的大小
template<int SIZE>
class FixBuffer {
public:

    FixBuffer():cur_(data_){}

    ~FixBuffer()= default;

    const char*data() const{return data_;}

    int length()const {return  static_cast<int>(cur_-data_);}

    char* current(){return cur_;}

    int avail() const {return static_cast<int>(end()-cur_);}

    void append(const char * buf,size_t len){
        if(static_cast<size_t>(avail())>len){
            //给字符地址初始化
            memcpy(cur_,buf,len);
            cur_ = cur_+len;
        }
    }

    void reset(){cur_ = data_;}

    void bezero(){bzero(data_,sizeof(data_));}

private:

    const char*end() const{return data_ + sizeof data_;}

    char data_[SIZE]{};//  buffer

    char*cur_;//这个是当前指针
};