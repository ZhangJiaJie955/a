#include "objectcontainer.h"
#include "objectwrapper.h"
#include "h9Log.h"

using namespace std;

namespace H9 {
ObjectContainer::~ObjectContainer()
{
    cleanup();
}

void ObjectContainer::cleanup()
{
    for (map<string, ObjectWrapper *>::iterator wit = m_wrappers.begin();
         wit != m_wrappers.end(); ++wit)
        delete wit->second;

    map<string, ObjectWrapper *>().swap(m_wrappers);
}

ObjectWrapper *ObjectContainer::getWrapper(string const &key) const
{
    map<string, ObjectWrapper *>::const_iterator wit = m_wrappers.find(key);
    if (m_wrappers.end() == wit)
        return NULL;
    return wit->second;
}

ObjectWrapper *ObjectContainer::addWrapper(string const &key)
{
    ObjectWrapper *tmp = getWrapper(key);
    if(tmp) {return tmp;}
    ObjectWrapper *wrapper = new ObjectWrapper;
    setWrapper(key, wrapper);
    return wrapper;
}

void ObjectContainer::setWrapper(string const &key, ObjectWrapper *wrapper)
{
    map<string, ObjectWrapper *>::iterator wit = m_wrappers.find(key);
    if (m_wrappers.end() != wit) {
        H9_WARN("WARNING: deleting the old object with key %s", key.c_str());
        delete wit->second;
    }
    m_wrappers[key] = wrapper;
}

void ObjectContainer::delWrapper(string const &key)
{
    map<string, ObjectWrapper *>::iterator wit = m_wrappers.find(key);
    if (m_wrappers.end() == wit)
        return;

    delete wit->second;
    m_wrappers.erase(wit);
}
}//H9    
