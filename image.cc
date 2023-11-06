#include "image.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "h9Log.h"

using namespace std;

namespace H9 {
bool ImageObject::readFromFile(string const &filename)
{
    m_data = cv::imread(filename);
    if (!m_data.data) {
        H9_ERROR("Failed to load image from %s", filename.c_str());
        return false;
    }
        
    m_colortype = BGR;
    H9_INFO("Loaded image channel: %d, depth: %d", m_data.channels(), m_data.depth());

    return true;
}

bool ImageObject::writeToFile(string const &filename) const
{
    if (BGR == m_colortype && CV_8U == m_data.depth() && 3 == m_data.channels())
        return cv::imwrite(filename, m_data);

    cv::Mat stdImg = cvtTo8UBGR();
    return cv::imwrite(filename, stdImg);
}

cv::Mat ImageObject::cvtTo8UBGR() const
{
    cv::Mat mat;
    if (GRAY == m_colortype && CV_8U == m_data.depth() && 1 == m_data.channels())
        cv::cvtColor(m_data, mat, cv::COLOR_GRAY2BGR);
    else if (GRAY == m_colortype && CV_32F == m_data.depth() && 1 == m_data.channels()) {
        double minval, maxval;
        cv::Mat tmp;
        cv::minMaxLoc(m_data, &minval, &maxval);
        m_data.convertTo(tmp, CV_8U, 255 / (maxval - minval), minval);
        cv::cvtColor(tmp, mat, cv::COLOR_GRAY2BGR);
    } else {
        H9_WARN("Cannot convert to 8UBGR for saving");
    }
    return mat;
}

int ImageObject::getColorConverstionCode(const ImageObject::ColorType& from, const ImageObject::ColorType& to)
{
	if (from == GRAY) {
		if (to == BGR) return cv::COLOR_GRAY2BGR;
		if (to == RGB) return cv::COLOR_GRAY2RGB;
		if (to == BGRA) return cv::COLOR_GRAY2BGRA;
		if (to == RGBA) return cv::COLOR_GRAY2RGBA;
		// if (to == HSV) return cv::COLOR_GRAY2HSV;
		// if (to == HLS) return cv::COLOR_GRAY2HLS;
		// if (to == YCrCb) return cv::COLOR_GRAY2YCrCb;
	}
	
	if (from == BGR) {
		if (to == GRAY) return cv::COLOR_BGR2GRAY;
		if (to == RGB) return cv::COLOR_BGR2RGB;
		if (to == HSV) return cv::COLOR_BGR2HSV;
		if (to == HLS) return cv::COLOR_BGR2HLS;
		if (to == YCrCb) return cv::COLOR_BGR2YCrCb;
		if (to == XYZ) return cv::COLOR_BGR2XYZ;
		if (to == Luv) return cv::COLOR_BGR2Luv;
		if (to == Lab) return cv::COLOR_BGR2Lab;
		if (to == RGBA) return cv::COLOR_BGR2RGBA;
		if (to == BGRA) return cv::COLOR_BGR2BGRA;
	}
	
	if (from == RGB) {
		if (to == GRAY) return cv::COLOR_RGB2GRAY;
		if (to == BGR) return cv::COLOR_RGB2BGR;
		if (to == HSV) return cv::COLOR_RGB2HSV;
		if (to == HLS) return cv::COLOR_RGB2HLS;
		if (to == YCrCb) return cv::COLOR_RGB2YCrCb;
		if (to == XYZ) return cv::COLOR_RGB2XYZ;
		if (to == Luv) return cv::COLOR_RGB2Luv;
		if (to == Lab) return cv::COLOR_RGB2Lab;
		if (to == RGBA) return cv::COLOR_RGB2RGBA;
		if (to == BGRA) return cv::COLOR_RGB2BGRA;
	}
	
	if (from == HSV) {
		// if (to == GRAY) return cv::COLOR_HSV2GRAY;
		if (to == BGR) return cv::COLOR_HSV2BGR;
		if (to == RGB) return cv::COLOR_HSV2RGB;
		// if (to == HLS) return cv::COLOR_HSV2HLS;
		// if (to == YCrCb) return cv::COLOR_HSV2YCrCb;
	}
	
	if (from == HLS) {
		// if (to == GRAY) return cv::COLOR_HLS2GRAY;
		if (to == BGR) return cv::COLOR_HLS2BGR;
		if (to == RGB) return cv::COLOR_HLS2RGB;
		// if (to == HSV) return cv::COLOR_HLS2HSV;
		// if (to == YCrCb) return cv::COLOR_HLS2YCrCb;
	}

	if (from == YCrCb) {
		// if (to == GRAY) return cv::COLOR_YCrCb2GRAY;
		if (to == BGR) return cv::COLOR_YCrCb2BGR;
		if (to == RGB) return cv::COLOR_YCrCb2RGB;
		// if (to == HSV) return cv::COLOR_YCrCb2HSV;
		// if (to == HLS) return cv::COLOR_YCrCb2HLS;
	}
	
	if (from == Luv) {
		if (to == BGR) return cv::COLOR_Luv2BGR;
		if (to == RGB) return cv::COLOR_Luv2RGB;
	}

	if (from == Lab) {
		if (to == BGR) return cv::COLOR_Lab2BGR;
		if (to == RGB) return cv::COLOR_Lab2RGB;
	}

	if (from == XYZ) {
		if (to == BGR) return cv::COLOR_XYZ2BGR;
		if (to == RGB) return cv::COLOR_XYZ2RGB;
	}

	if (from == RGBA) {
		if (to == BGR) return cv::COLOR_RGBA2BGR;
		if (to == RGB) return cv::COLOR_RGBA2RGB;
	}

	if (from == BGRA) {
		if (to == BGR) return cv::COLOR_BGRA2BGR;
		if (to == RGB) return cv::COLOR_BGRA2RGB;
	}

	return -100000;
}
	
	
	
bool ImageObject::cvtColorFrom(ImageObject const &src)
{
    int code = getColorConverstionCode(src.m_colortype, m_colortype);
	
	if (code < 0)
	{
		int code1 = getColorConverstionCode(src.m_colortype, BGR);
		int code2 = getColorConverstionCode(BGR, src.m_colortype);
		if (code1 < 0 || code2 <0) {
			H9_ERROR("This conversion is not supported yet");
			return false;
		}
		
		cv::Mat m;
		cv::cvtColor(src.m_data, m, code1);
		cv::cvtColor(m, m_data, code2);
		return true;
    }
	
    cv::cvtColor(src.m_data, m_data, code);
    return true;
}

ImageObject::ColorType ImageObject::getColorType(string const &type)
{
    if ("GRAY" == type)
        return GRAY;
    if ("BGR" == type)
        return BGR;
    if ("RGB" == type)
        return RGB;
    if ("HSV" == type)
        return HSV;
	if ("HSL" == type || "HLS" == type)
		return HLS;
	if ("Luv" == type)
		return Luv;
	if ("Lab" == type)
		return Lab;
	if ("XYZ" == type)
		return XYZ;
	if ("YCrCb" == type || "YCbCr" == type)
		return YCrCb;
	if ("BGRA" == type)
		return BGRA;
	if ("RGBA" == type)
		return RGBA;
    H9_WARN("unknown color type %s", type.c_str());
    return INVALID;
}
}//H9
