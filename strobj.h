#ifndef _STROBJ_H
#define _STROBJ_H
#include <string>

namespace H9 {
class StringObject
{
public:
    StringObject() : m_data("") {}
    StringObject(std::string const &str= "") : m_data(str) {}
    ~StringObject() {}
    
    void clone(StringObject const &src)
    {
        m_data = src.m_data;
    }
public:
    std::string m_data;
};

}//H9
#endif
