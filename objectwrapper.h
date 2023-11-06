#ifndef _OBJECTWRAPPER_H
#define _OBJECTWRAPPER_H
#include <cstdint>
#include "image.h"
#include "strobj.h"

namespace H9 {
class ObjectWrapper
{
public:
    enum ObjectType {IMAGE = 0x1,
                     STR = 0x2,
    };

public:
    ObjectWrapper() : m_validtypes(0), m_str("") {}
    ~ObjectWrapper() {}

    bool hasValidType(ObjectType type) const {return m_validtypes & type;}
    void setValidType(ObjectType type) {m_validtypes |= type;}
    void invalidateType(ObjectType type) {m_validtypes &= ~type;}

    ImageObject &getImageObject() {return m_image;}
    ImageObject const &getImageObject() const {return m_image;}
    
    StringObject &getStringObject() {return m_str;}
    StringObject const &getStringObject() const {return m_str;}

private:
    uint64_t m_validtypes;
    ImageObject m_image;
    StringObject m_str;
};
}
#endif
