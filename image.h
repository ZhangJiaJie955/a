#ifndef _IMAGE_H
#define _IMAGE_H
#include <opencv2/core/core.hpp>
#include <string>

namespace H9 {
class ImageObject
{
public:
    enum ColorType {GRAY = 0,
                    BGR,
                    RGB,
                    HSV,
					HLS,
                    XYZ,
                    Luv,
                    Lab,
					YCrCb,
                    BGRA,
                    RGBA,
		    INVALID
    };

public:
    static ColorType getColorType(std::string const &);
	static int getColorConverstionCode(const ColorType&, const ColorType&);
public:
    ImageObject() : m_colortype(INVALID) {}
    ImageObject(cv::Mat const& d_, ColorType t_ = BGR) : m_colortype(t_), m_data(d_) {}
    ~ImageObject() {}

    bool readFromFile(std::string const &);
    bool writeToFile(std::string const &) const;

    bool cvtColorFrom(ImageObject const &);

    void setColorType(std::string const &type) {m_colortype = getColorType(type);}
    void setColorType(ColorType type) {m_colortype = type;}
    ColorType colorType() const { return m_colortype;}
    
    void clone(ImageObject const &src)
    {
        m_colortype = src.m_colortype;
        m_data = src.m_data.clone();
    }

    void cloneAs8UBGR(ImageObject const &src)
    {
        m_colortype = BGR;
        if (BGR == src.m_colortype)
            m_data = src.m_data.clone();
        else
            m_data = src.cvtTo8UBGR();
    }

private:
    cv::Mat cvtTo8UBGR() const;

public:
    ColorType m_colortype;
    cv::Mat m_data;
};

}//H9
#endif
