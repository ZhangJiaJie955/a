#ifndef _COLORPREDICTOR_H
#define _COLORPREDICTOR_H
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using std::string;
using std::vector;

namespace YAML
{
    class Node;
}

namespace H9
{
class ImageObject;
class ObjectContainer;

class ColorPredictor
{
public:
    ColorPredictor()  
    : m_offset_left(0.0), 
      m_offset_right(0.0) {
          m_color_ranges["Luv"] = vector<vector<double>>(3, vector<double>(2, 0.0));
          m_color_ranges["Lab"] = vector<vector<double>>(3, vector<double>(2, 0.0));
          m_color_ranges["HSV"] = vector<vector<double>>(3, vector<double>(2, 0.0));
          m_color_ranges["YCrCb"] = vector<vector<double>>(3, vector<double>(2, 0.0));
      }
    ~ColorPredictor() {}

    bool predict(ImageObject const &, ImageObject const &, vector<ImageObject*> &);
    vector<vector<double>>& get_chosen_range(int);
    string get_chosen_colorspace(int i) {return m_qualities[i].first;}
    void set_offset(vector<double> const &);

private:
    void predict_color_range(cv::Mat const &, cv::Mat const &, vector<vector<double>> &);
    void apply_offset(vector<vector<double>> &);
    string code_str(int);
    std::map<string, vector<vector<double>>> m_color_ranges;
    vector<std::pair<string, int>> m_qualities;
    std::map<string, cv::Mat> m_pillmask;
    const int m_code[4] = { cv::COLOR_BGR2Luv, cv::COLOR_BGR2Lab, cv::COLOR_BGR2HSV, cv::COLOR_BGR2YCrCb };
    double m_offset_left;
    double m_offset_right;
};
} //H9
#endif
