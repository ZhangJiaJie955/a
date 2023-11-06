#include "ColorPredictor.h"
#include <iostream>
#include <yaml-cpp/yaml.h>
#include "h9Log.h"
#include "h9Timer.h"
#include "image.h"


using namespace std;

namespace H9
{
string ColorPredictor::code_str(int code)
{
    switch (code)
	{
    case cv::COLOR_BGR2Luv:
        return "Luv";
    case cv::COLOR_BGR2Lab:
        return "Lab";
    case cv::COLOR_BGR2HSV:
        return "HSV";
    case cv::COLOR_BGR2YCrCb:
        return "YCrCb";
	default:
		break;
	}
    return "";
}

void ColorPredictor::set_offset(vector<double> const &offsets)
{
    m_offset_left = offsets[0];
    m_offset_right = offsets[1];
}

void ColorPredictor::predict_color_range(cv::Mat const &src, cv::Mat const &mask, vector<vector<double>> &color_range)
{
    vector<vector<double>>(3, vector<double>(2)).swap(color_range);
    vector<cv::Mat> src_vec;
    split(src, src_vec);
    size_t nchannels = src_vec.size();
    cv::Point minLoc, maxLoc;
    for (size_t i = 0; i < nchannels; i++) {
        minMaxLoc(src_vec[i], &color_range[i][0], &color_range[i][1], &minLoc, &maxLoc, mask);
    }
}

void ColorPredictor::apply_offset(vector<vector<double>> &color_range)
{
    for (size_t i = 0; i < 3; i++) {
		vector<double> &range = color_range[i];
		double dist = range[1] - range[0];
		range[0] -= dist / 2 * m_offset_left;
		range[1] += dist / 2 * m_offset_right;
	}
}

bool ColorPredictor::predict(ImageObject const& src, ImageObject const& mask, vector<ImageObject*> & vec_dst)
{
    size_t iteration_times = sizeof(m_code) / sizeof(m_code[0]);
    for (size_t i = 0; i < iteration_times; i++) {
        string str = code_str(m_code[i]);
        vector<vector<double>> &color_range = m_color_ranges[str];
        cv::Mat src_cvt;
        cv::cvtColor(src.m_data, src_cvt, m_code[i]);
        predict_color_range(src_cvt, mask.m_data, color_range);
        apply_offset(color_range);
	    cv::Scalar lb(color_range[0][0], color_range[1][0], color_range[2][0]);
	    cv::Scalar ub(color_range[0][1], color_range[1][1], color_range[2][1]);
        cv::Mat pill_mask;
        cv::inRange(src_cvt, lb, ub, pill_mask);
        vector<vector<cv::Point>> contours;
        cv::findContours(pill_mask, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
        m_qualities.push_back(make_pair(str, contours.size()));
        m_pillmask[str] = pill_mask;
    }
    std::sort(m_qualities.begin(), m_qualities.end());
    // std::sort(m_qualities.begin(), m_qualities.end(), [](const pair<string, int>& lhs, const pair<string, int>& rhs){return lhs.second < rhs.second;});
    size_t ndsts = vec_dst.size();
    for (size_t i = 0; i < ndsts; i++) {
        string str = m_qualities[i].first;
        vec_dst[i]->m_data = m_pillmask[str];
    }
    //
    // ostringstream oss;
    // oss << m_qualities[0].first << ": " << m_qualities[0].second << endl;
    // oss << m_qualities[1].first << ": " << m_qualities[1].second << endl;
    // oss << m_qualities[2].first << ": " << m_qualities[2].second << endl;
    // oss << m_qualities[3].first << ": " << m_qualities[3].second << endl;
    // H9_INFO(oss.str().c_str());
    //
    return true;
}

vector<vector<double>>& ColorPredictor::get_chosen_range(int choice)
{
    string str = m_qualities[choice].first;
    return m_color_ranges[str];
}

}