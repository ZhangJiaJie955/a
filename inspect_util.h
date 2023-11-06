#ifndef _INSPECTUTIL_H
#define _INSPECTUTIL_H
#include <vector>
#include <string>
#include <map>
#include <yaml-cpp/yaml.h>
#include <opencv2/opencv.hpp>
#include "h9Log.h"
namespace H9 {

#if CV_VERSION_MAJOR < 4
#define H9_SCHARR CV_SCHARR
#define MAX_SOBEL_KSIZE CV_MAX_SOBEL_KSIZE
#else
#define H9_SCHARR cv::FILTER_SCHARR
#define MAX_SOBEL_KSIZE 7
#endif

template<typename T>
void parseOneList(YAML::Node const &node, std::vector<T> &ret)
{
    std::vector<T>().swap(ret);

    if (node.IsSequence())
        for (size_t i = 0; i < node.size(); ++i)
            ret.push_back(node[i].as<T>());
}

template<typename T>
void parse2DList(YAML::Node const &node, std::vector<std::vector<T> > &ret)
{
    std::vector<std::vector<T> >().swap(ret);

    if (node.IsSequence()) {
        ret.resize(node.size());
        for (size_t i = 0; i != node.size(); ++i)
            parseOneList(node[i], ret[i]);
    }
}

template<typename T>
void parseParamsMap(YAML::Node const &node, std::map<std::string, T> &ret)
{
    std::map<std::string, T>().swap(ret);
    if (!node.IsMap())
        return;
    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
        ret[it->first.as<std::string>()] = it->second.as<double>();
}

inline cv::Point parsePoint(YAML::Node const &node)
{
    return cv::Point(node[0].as<int>(), node[1].as<int>());
}

template<typename T>
inline void setSize2Node(YAML::Node &node, cv::Size_<T> const &sz, YAML::EmitterStyle::value style = YAML::EmitterStyle::Flow)
{
    node.SetStyle(style);
    node.push_back(sz.width);
    node.push_back(sz.height);
}
template<typename T>
inline void setPoint2Node(YAML::Node &node, cv::Point_<T> const &pt, YAML::EmitterStyle::value style = YAML::EmitterStyle::Flow)
{
    node.SetStyle(style);
    node.push_back(pt.x);
    node.push_back(pt.y);
}
inline void setRect2Node(YAML::Node &node, cv::Rect const &r, YAML::EmitterStyle::value style = YAML::EmitterStyle::Flow)
{
    node.SetStyle(style);
    node.push_back(r.x);
    node.push_back(r.y);
    node.push_back(r.width);
    node.push_back(r.height);
}
inline void pushRect2Node(YAML::Node node, cv::Rect const & r)
{
    YAML::Node rnode;
    // rnode.SetStyle(YAML::EmitterStyle::Flow);
    setRect2Node(rnode, r);
    // rnode.push_back(r.x);
    // rnode.push_back(r.y);
    // rnode.push_back(r.width);
    // rnode.push_back(r.height);
    node.push_back(rnode);
}

inline bool isSquare(const std::vector<cv::Point>& contour)
{
    auto rrect = cv::minAreaRect(contour);
    float aspect_ratio =  rrect.size.width / static_cast<double>(rrect.size.height);
    H9_INFO("mark aspect_ratio: %f", aspect_ratio);
    if (aspect_ratio < 0.618 && aspect_ratio > 1.618)
        return false;
    std::vector<cv::Point> hull;
    cv::convexHull(contour, hull);
    double area1 = cv::contourArea(contour);
    double area2 = rrect.size.area();
    float area_ratio = area1 / area2;
    H9_INFO("mark area_ratio: %f", area_ratio);
    if (area_ratio < 0.8)
        return false;
    return true;
}


inline double get_det_double(YAML::Node const &node)
{
    if (node.IsScalar())
        return node.as<double>();
    return node["val"].as<double>();
}

inline int get_det_int(YAML::Node const &node)
{
    if (node.IsScalar())
        return node.as<int>();
    return node["val"].as<int>();
}

inline double dist_sq(cv::Point2f const &p, cv::Point2f const &q)
{
    double diff_x = p.x - q.x;
    double diff_y = p.y - q.y;
    return diff_x * diff_x + diff_y * diff_y;
}

inline double dist_sq(cv::Point const &p, cv::Point const &q)
{
    double diff_x = p.x - q.x;
    double diff_y = p.y - q.y;
    return diff_x * diff_x + diff_y * diff_y;
}

template<typename T>
inline double dist_l1(std::vector<T> const &p1, std::vector<T> const &p2)
{
    if (p1.size() != p2.size())
        return -1;
    double sum = 0;
    for (size_t i = 0; i != p1.size(); ++i)
        sum += std::abs(p1[i] - p2[i]);
    return sum;
}

inline cv::Point get_contour_pos(std::vector<cv::Point> const &contour)
{
    size_t npoints = contour.size();
    cv::Point p1(contour[rand() & npoints]);
    cv::Point p2(contour[rand() & npoints]);
    cv::Point p3(contour[rand() & npoints]);
    return (p1 + p2 + p3) / 3;
}

template<typename T>
inline bool cmp_size_bigger(std::vector<T> const &lhs, std::vector<T> const &rhs)
{
    return lhs.size() > rhs.size();
}

template<typename T>
inline bool cmp_size_smaller(std::vector<T> const &lhs, std::vector<T> const &rhs)
{
    return lhs.size() < rhs.size();
}

inline bool isEven(int n)
{
  return n % 2 == 0;
}

inline bool isValidROI(cv::Size size, cv::Rect roi)
{
    return (cv::Rect(0, 0, size.width, size.height) & roi) == roi;
}

inline void makeValidROI(cv::Size size, cv::Rect& roi)
{
    roi &= cv::Rect(0, 0, size.width, size.height);
}
/**
 * @description: from the nearest point in contour to pt
 * @param {Point const} &pt input target point
 * @param {vector<cv::Point>} const &contour input source points
 * @return {int} the idx of the nearest point in contour
 */ 
int findNearestPointIdx(cv::Point const &pt, std::vector<cv::Point> const &contour);
/**
 * @description: find the left point of a contour
 * @param {vector<cv::Point>} const & contour input point set
 * @return {*} the iterator of result point
 */
std::vector<cv::Point>::const_iterator findLeftPoint(std::vector<cv::Point> const& contour);
/**
 * @description: find the right point of a contour
 * @param {vector<cv::Point>} const & contour input point set
 * @return {*} the iterator of result point
 */
std::vector<cv::Point>::const_iterator findRightPoint(std::vector<cv::Point> const& contour);
/**
 * @description: find the top point of a contour
 * @param {vector<cv::Point>} const & contour input point set
 * @return {*} the iterator of result point
 */
std::vector<cv::Point>::const_iterator findTopPoint(std::vector<cv::Point> const &contour);
/**
 * @description: find the top point of a contour
 * @param {vector<cv::Point>} const & contour input point set
 * @return {*} the iterator of result point
 */
std::vector<cv::Point>::const_iterator findBottomPoint(std::vector<cv::Point> const &contour);
/**
 * @description: find the largest area contour by cv::contourArea
 * @param {vector<std::vector<cv::Point>>} const input contours
 * @return {*} the interator of result contour
 */
std::vector<std::vector<cv::Point>>::const_iterator findBiggestContour(std::vector<std::vector<cv::Point>> const &contours);
/**
 * @description: find the tallest contour by cv::boundingRect.height
 * @param {vector<std::vector<cv::Point>>} const input contours
 * @return {*} the interator of result contour
 */
std::vector<std::vector<cv::Point>>::const_iterator findHighestContour(std::vector<std::vector<cv::Point>> const &contours);

/**
 * @description: morphology close operation with BorderType cv::BORDER_CONSTANT and value cv::Scalar(0)
 * @param {Mat const} &src input image
 * @param {Mat} &dst output image
 * @param {Size} sz close kernel size
 * @return {*}
 */
void border0Close(cv::Mat const &src, cv::Mat &dst, cv::Size sz);
/**
 * @description: overloaded function. use directly kernel not size
 * @param {Mat const} &src
 * @param {Mat} &dst
 * @param {Mat const} &kernel
 * @return {*}
 */
void border0Close(cv::Mat const &src, cv::Mat &dst, cv::Mat const &kernel);

bool computeCapsuleBBoxCorners(cv::RotatedRect const &, std::vector<int> &);
cv::Mat computeCapsuleBBoxTrans(cv::RotatedRect const &, std::vector<int> const &);
cv::Mat computeCapsuleBBoxTrans(cv::Rect const &, std::vector<int> const &);

class QRCodeFinder
{
private:
    double angle(cv::Point const &, cv::Point const &, cv::Point const &);
    cv::Mat m_src;                  // input image
    int m_code_unit_width;          // control the morphology kernel size and poly approximation accuracy 
    int m_code_amount;              // total amount of QR Code, only support 1 QRCode now
    double m_code_estimated_area;   // the rough estimated area of target QRCode 
    double m_max_aspect_ratio;      // restrict the max aspect ratio of QRCode
    double m_max_cosine;            // restrict the max cosine of each angle of QRCode
    double m_area_ratio;
    int m_debug_level;
    bool m_fill_holes_flag;
    cv::Size m_open_sz;
    cv::Size m_close_sz;
    std::string m_base_name;
    std::vector<std::vector<cv::Point>> m_contours;
public:
    QRCodeFinder() {}
    QRCodeFinder(int code_unit_width, double code_estimated_area, int code_amount = 1, float max_aspect_ratio = 1.2, float max_cosine = 0.07, float area_ratio = 0.9)
      : m_code_unit_width(code_unit_width),
        m_code_amount(code_amount),
        m_code_estimated_area(code_estimated_area),
        m_max_aspect_ratio(max_aspect_ratio),
        m_max_cosine(max_cosine),
        m_area_ratio(area_ratio),
        m_debug_level(2),
        m_fill_holes_flag(false),
        m_open_sz(cv::Size(code_unit_width, code_unit_width*3)),
        m_base_name("") {}
    ~QRCodeFinder() {}
    /**
     * @description: set input image, only CV_8UC1 format allowed
     * @param {Mat const} &img input image
     * @return {*}
     */
    void setInput(cv::Mat const &img) {
        assert(img.type()==CV_8UC1);
        img.copyTo(m_src);
    }
    void setDebugLevel(int level) {
        m_debug_level = level;
    }
    void setBaseName(std::string str) {
        m_base_name = str;
    }
    void setOpenSize(cv::Size sz) {
        m_open_sz = sz;
    }
    void setCloseSize(cv::Size sz) {
        m_close_sz = sz;
    }
    void setFillHolesFlag(bool flag) {
        m_fill_holes_flag = flag;
    }
    bool find(std::vector<cv::Point> &vertices);
    bool find_bymark(std::vector<cv::Point2f> &vertices);
    std::vector<std::vector<cv::Point>> getContours() {return m_contours;}
};

class MSERFinder
{
private:
    int m_start_value;
    int m_end_value;
    int m_step;
    double m_res_thresh;
    int m_min_increment;

    cv::Mat m_binary;
public:
    MSERFinder(int start, int end, int step)
      : m_start_value(start),
        m_end_value(end),
        m_step(step) 
    {
        m_min_increment = INT_MAX;
        m_res_thresh = m_start_value;
    }
    ~MSERFinder() {}
    bool find(cv::Mat const & img);
    double getResThresh() {return m_res_thresh - 0.5;}
};


} //H9

namespace YAML {
template<>
struct convert<cv::Rect> {
  static Node encode(const cv::Rect& rhs) {
    Node node;
    node.push_back(rhs.x);
    node.push_back(rhs.y);
    node.push_back(rhs.width);
    node.push_back(rhs.height);
    return node;
  }

  static bool decode(const Node& node, cv::Rect& rhs) {
    if(!node.IsSequence() || node.size() != 4) {
      return false;
    }

    rhs.x = node[0].as<int>();
    rhs.y = node[1].as<int>();
    rhs.width = node[2].as<int>();
    rhs.height = node[3].as<int>();
    return true;
  }
};

template<>
struct convert<cv::Size> {
  static Node encode(const cv::Size& rhs) {
    Node node;
    node.push_back(rhs.width);
    node.push_back(rhs.height);
    return node;
  }

  static bool decode(const Node& node, cv::Size& rhs) {
    if(!node.IsSequence() || node.size() != 2) {
      return false;
    }

    rhs.width = node[0].as<int>();
    rhs.height = node[1].as<int>();
    return true;
  }
};

template<>
struct convert<cv::Range> {
  static Node encode(const cv::Range& rhs) {
    Node node;
    node.push_back(rhs.start);
    node.push_back(rhs.end);
    return node;
  }

  static bool decode(const Node& node, cv::Range& rhs) {
    if(!node.IsSequence() || node.size() != 2) {
      return false;
    }

    rhs.start = node[0].as<int>();
    rhs.end = node[1].as<int>();
    return true;
  }
};

template<typename T>
struct convert<cv::Point_<T>> {
  static Node encode(const cv::Point_<T>& rhs) {
    Node node;
    node.push_back(rhs.x);
    node.push_back(rhs.y);
    return node;
  }

  static bool decode(const Node& node, cv::Point_<T>& rhs) {
    if(!node.IsSequence() || node.size() != 2) {
      return false;
    }

    rhs.x = node[0].as<T>();
    rhs.y = node[1].as<T>();
    return true;
  }
};

template<typename T, int cn>
struct convert<cv::Vec<T, cn>> {
  static Node encode(const cv::Vec<T, cn>& rhs) {
    Node node;
    for (size_t i = 0; i < cn; i++)
      node.push_back(rhs[i]);
    return node;
  }

  static bool decode(const Node& node, cv::Vec<T, cn>& rhs) {
    if(!node.IsSequence() || node.size() != cn) {
      return false;
    }
    for (size_t i = 0; i < cn; i++)
      rhs[i] = node[i].as<T>();
  
    return true;
  }
};

}
#endif
