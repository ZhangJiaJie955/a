#ifndef _DEFECT_DATA_H
#define _DEFECT_DATA_H
#include <string>
#include "image.h"

namespace H9
{

struct DefectData
{
    DefectData()
        : m_isDefect(false), m_basename(""), m_defectType("Good"), m_srcImg(NULL), m_defImg(NULL), m_imgInited(false)
    {}
    ~DefectData() {}

    void setDefectType(std::string const &deftype)
    {
        m_defectType = deftype;
    }

    std::string getDefectStr() const {return m_defectType;}

    void appendDefectType(std::string const &deftype)
    {
        m_defectType = m_defectType + "\n" + deftype;
    }

    void prepareImg()
    {
        m_defImg->cloneAs8UBGR(*m_srcImg);
        m_imgInited = true;
    }

    bool defImgInited() const {return m_imgInited;}
    bool isDefect() const {return m_isDefect;}
    std::string const &getDefectType() const {return m_defectType;}

    bool m_isDefect;
    std::string m_basename;
    std::string m_defectType;
    ImageObject const *m_srcImg;
    ImageObject *m_defImg;
    bool m_imgInited;
};

struct CapsuleDefect
{
public:
    CapsuleDefect()
        : m_basename(""), m_srcImg(NULL), m_defImg(NULL)
    {}
    ~CapsuleDefect() {}

    void config(int num, std::string const &basename, ImageObject const *src, ImageObject *dst)
    {
        m_basename = basename;
        m_srcImg = src;
        m_defImg = dst;

        std::vector<bool>(num, false).swap(m_isDefect);
        std::vector<std::string>(num, "Good").swap(m_defType);
    }

    bool setDefect(int i, std::string const &def)
    {
        if (i < (int)m_isDefect.size()) {
            m_isDefect[i] = true;
            m_defType[i] = def;
            return true;
        }
        return false;
    }

    bool setNA(int i)
    {
        if (i < (int)m_isDefect.size()) {
            m_defType[i] = "NA";
            return true;
        }
        return false;
    }

    void prepareImg() {m_defImg->cloneAs8UBGR(*m_srcImg);}

    bool isDefect() const
    {
        for (std::vector<bool>::const_iterator iit = m_isDefect.begin();
                iit != m_isDefect.end(); ++iit)
            if (*iit)
                return true;
        return false;
    }

    std::vector<std::string> const &getDefType() const {return m_defType;}

public:
    std::string m_basename;
    ImageObject const *m_srcImg;
    ImageObject *m_defImg;

    std::vector<bool> m_isDefect;
    std::vector<std::string> m_defType;
};
} //H9
#endif
