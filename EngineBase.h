#ifndef _ENGINE_BASE_H
#define _ENGINE_BASE_H

namespace YAML
{
    class Node;
}

namespace H9 {

class EngineBase
{
public:
    EngineBase() {}
    EngineBase(std::string const &opstr) {}
    virtual ~EngineBase() {}

    virtual bool init(YAML::Node const &) = 0;

};


}//H9

#endif
