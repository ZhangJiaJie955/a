#ifndef _OBJECTCONTAINER_H
#define _OBJECTCONTAINER_H
#include <string>
#include <map>

namespace H9 {
class ObjectWrapper;
class ObjectContainer
{
public:
    ObjectContainer() {}
    ~ObjectContainer();

    ObjectWrapper *getWrapper(std::string const &) const;
    ObjectWrapper *addWrapper(std::string const &);
    void setWrapper(std::string const &, ObjectWrapper *);

    bool hasWrapper(std::string const &key) const
    {
        return m_wrappers.find(key) != m_wrappers.end();
    }
    void delWrapper(std::string const &);

    void cleanup();

    int size()
    {
	return m_wrappers.size();
    }

private:
    std::map<std::string, ObjectWrapper *> m_wrappers;
};
}//H9
#endif
