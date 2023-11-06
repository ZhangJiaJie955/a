/*
 * @Author: your name
 * @Date: 2021-06-23 11:28:24
 * @LastEditTime: 2022-04-15 19:12:30
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \hlzn_package\src\algo\inspect_util.cc
 */
#include "inspect_util.h"
#include <vector>
#include <opencv2/imgproc/imgproc.hpp>
#include "h9Log.h"
#include "h9Timer.h"
#undef max
#undef min
using namespace std;
using cv::Point;
namespace H9 {

int findNearestPointIdx(cv::Point const &pt, std::vector<cv::Point> const &contour)
{
    H9_TIMER(__func__);
    double min_dist = 999999;
    int idx = -1;
    size_t npoints = contour.size();
    for (size_t i = 0; i < npoints; i++) {
        double dist = dist_sq(pt, contour[i]);
        if (dist < min_dist) {
            min_dist = dist;
            idx = i;
        }
    }
    return idx;
}
//2022/10/06_wjl_新增findeLeftPoint、findRightPoint
std::vector<cv::Point>::const_iterator findLeftPoint(std::vector<cv::Point> const& contour)
{
    return min_element(contour.begin(), contour.end(), [](Point const& lhs, Point const& rhs)
        { return lhs.x < rhs.x; });
}
std::vector<cv::Point>::const_iterator findRightPoint(std::vector<cv::Point> const& contour)
{
    return max_element(contour.begin(), contour.end(), [](Point const& lhs, Point const& rhs)
        { return lhs.x < rhs.x; });
}
//end
std::vector<cv::Point>::const_iterator findTopPoint(std::vector<cv::Point> const &contour)
{
    return min_element(contour.begin(), contour.end(), [](Point const &lhs, Point const &rhs)
                          { return lhs.y < rhs.y; });
}

std::vector<cv::Point>::const_iterator findBottomPoint(std::vector<cv::Point> const &contour)
{
    return max_element(contour.begin(), contour.end(), [](Point const &lhs, Point const &rhs)
                          { return lhs.y < rhs.y; });
}

vector<vector<Point>>::const_iterator findBiggestContour(vector<vector<Point>> const &contours)
{
    return max_element(contours.begin(), contours.end(), [](vector<Point> const &lhs, vector<Point> const &rhs)
                          { return cv::contourArea(lhs) < cv::contourArea(rhs);});
}

vector<vector<Point>>::const_iterator findHighestContour(vector<vector<Point>> const &contours)
{
    return max_element(contours.begin(), contours.end(), [](vector<Point> const &lhs, vector<Point> const &rhs)
                       { return cv::boundingRect(lhs).height < cv::boundingRect(rhs).height; });
}

void border0Close(cv::Mat const &src, cv::Mat &dst, cv::Size sz)
{
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, sz);
    int dh = sz.width;
    int dv = sz.height;
    cv::copyMakeBorder(src, dst, dv, dv, dh, dh, cv::BORDER_CONSTANT, cv::Scalar(0));
    cv::morphologyEx(dst, dst, cv::MORPH_CLOSE, kernel);
    dst = dst(cv::Rect(dh, dv, src.cols, src.rows));
}

void border0Close(cv::Mat const &src, cv::Mat &dst, cv::Mat const &kernel)
{
    int dh = kernel.cols;
    int dv = kernel.rows;
    cv::Mat tmp;
    cv::copyMakeBorder(src, tmp, dv, dv, dh, dh, cv::BORDER_CONSTANT, cv::Scalar(0));
    cv::morphologyEx(tmp, tmp, cv::MORPH_CLOSE, kernel);
    dst = tmp(cv::Rect(dh, dv, src.cols, src.rows)).clone();
}

bool computeCapsuleBBoxCorners(cv::RotatedRect const &rect, vector<int> &corners)
{
    vector<cv::Point2f> vtx(4);
    rect.points(&vtx[0]);
    int start = dist_sq(vtx[0], vtx[3]) < dist_sq(vtx[0], vtx[1]) ? 1 : 2;
    if (dist_sq(vtx[start], vtx[start - 1]) < dist_sq(vtx[start], vtx[start + 1]))
        return false;

    double theta = atan2(vtx[start + 1].y - vtx[start].y, vtx[start + 1].x - vtx[start].x);
    H9_INFO("theta is calculated to be %f", theta);
    cv::Mat trans = cv::getRotationMatrix2D(rect.center, theta * 180 / CV_PI, 1);
    vector<cv::Point2f> bbox(4);
    cv::transform(vtx, bbox, trans);
    vector<int>(4, 0).swap(corners);
    corners[0] = (numeric_limits<int>::max)();
    corners[1] = (numeric_limits<int>::max)();
    corners[2] = (numeric_limits<int>::min)();
    corners[3] = (numeric_limits<int>::min)();
    for (int i = 0; i != 4; ++i) {
        if (bbox[i].x < corners[0])
            corners[0] = bbox[i].x;
        if (bbox[i].x > corners[2])
            corners[2] = bbox[i].x;
        if (bbox[i].y < corners[1])
            corners[1] = bbox[i].y;
        if (bbox[i].y > corners[3])
            corners[3] = bbox[i].y;
    }
    return true;
}

cv::Mat computeCapsuleBBoxTrans(cv::RotatedRect const &rect, vector<int> const &corner)
{
    vector<cv::Point2f> targets(3);
    targets[0] = cv::Point2f(corner[0], corner[3]);
    targets[1] = cv::Point2f(corner[0], corner[1]);
    targets[2] = cv::Point2f(corner[2], corner[1]);
    vector<cv::Point2f> vs(4);
    rect.points(&vs[0]);
    int start = 0;
    if (dist_sq(vs[0], vs[3]) > dist_sq(vs[0], vs[1]))
        start = 1;

    return cv::getAffineTransform(&vs[start], &targets[0]);
}

cv::Mat computeCapsuleBBoxTrans(cv::Rect const &rect, vector<int> const &corner)
{
    vector<cv::Point2f> targets(3);
    targets[0] = cv::Point2f(corner[0], corner[3]);
    targets[1] = cv::Point2f(corner[0], corner[1]);
    targets[2] = cv::Point2f(corner[2], corner[1]);
    vector<cv::Point2f> vs(3);
    vs[0] = cv::Point2f(rect.x, rect.y + rect.height);
    vs[1] = cv::Point2f(rect.x, rect.y);
    vs[2] = cv::Point2f(rect.x + rect.width, rect.y);

    return cv::getAffineTransform(&vs[0], &targets[0]);
}

/**
 * @description: find the QR Code
 * @param {*}
 * @return {*} true if found, false if not found
 */
bool QRCodeFinder::find(std::vector<cv::Point> &vertices)
{
    H9_TIMER("QRCodeFinder::find()");
    double minVal, maxVal;
    cv::minMaxLoc(m_src, &minVal, &maxVal);
    cv::Mat bin;
    cv::threshold(m_src, bin, 0.618 * (minVal + maxVal), 255, cv::THRESH_BINARY_INV);
    cv::adaptiveThreshold(m_src, m_src, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, m_code_unit_width, 7);
    // cv::threshold(m_src, m_src, 0.5 * (minVal + maxVal), 255, cv::THRESH_BINARY_INV && cv::THRESH_OTSU);
    if (m_debug_level < 1) {
        cv::imwrite(m_base_name + "-qrcode-bin.png", m_src);
    }
    cv::medianBlur(m_src, m_src, 3);
    cv::bitwise_and(m_src, bin, m_src);
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, m_close_sz);
    cv::morphologyEx(m_src, m_src, cv::MORPH_CLOSE, element);
    if (m_debug_level < 1) {
        cv::imwrite(m_base_name + "-qrcode-close.png", m_src);
    }
    vector<vector<Point>> contours;
    cv::findContours(m_src, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    if (contours.empty())
        return false;
    if (m_fill_holes_flag) {
        cv::drawContours(m_src, contours, -1, cv::Scalar(255), -1);
    }
    cv::Mat element_open = cv::getStructuringElement(cv::MORPH_RECT, m_open_sz);
    cv::morphologyEx(m_src, m_src, cv::MORPH_OPEN, element_open);
    if (m_debug_level < 1) {
        cv::imwrite(m_base_name + "-qrcode-open.png", m_src);
    }
    cv::findContours(m_src, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    if (contours.empty())
        return false;
    m_contours = contours;
    size_t ncontours = contours.size();
    vector<int> sortIdx(ncontours);
    for (size_t i = 0; i < ncontours; i++) {
        sortIdx[i] = i;
    }
    std::sort(sortIdx.begin(), sortIdx.end(), [&contours](int a, int b){return contours[a].size() > contours[b].size();});
    vector<Point> approx;
    cv::Rect r;
    for (size_t i = 0; i < ncontours; i++) {
        // H9_PROFILER("QRCode Loop");
        vector<Point> const & contour = contours[sortIdx[i]];
        double area = cv::contourArea(contour);
        if (area < 0.67 * m_code_estimated_area || area > 1.5 * m_code_estimated_area) {
            H9_WARN("cannot find QRCode, due to area=%f", area);
            continue;
        }
        r = cv::boundingRect(contour);
        if (r.width > r.height * m_max_aspect_ratio || r.height > r.width * m_max_aspect_ratio) {
            H9_WARN("cannot find QRCode, due to r.width=%d, due to r.height=%d", r.width, r.height);
            continue;
        }
        vector<Point> hull;
        cv::convexHull(contour, hull);
        cv::approxPolyDP(hull, approx, m_code_unit_width, true);
        if (m_debug_level < 1) {
            cv::Mat canvas;
            cv::cvtColor(m_src, canvas, cv::COLOR_GRAY2BGR);
            cv::polylines(canvas, approx, true, cv::Scalar(0,0,255), 2);
            cv::imwrite(m_base_name + "-qrcode-res" + to_string(i) + ".png", canvas);
        }
        if (approx.size() != 4) {
            H9_WARN("cannot find QRCode, due to approx.size()=%d", approx.size());
            continue;
        }
        double max_cosine = 0;
        for (int j = 2; j < 5; j++) {
            // find the maximum cosine of the angle between joint edges
            double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));
            max_cosine = MAX(max_cosine, cosine);
        }
        double poly_area = cv::contourArea(approx);
        double area_ratio = area / poly_area;
        // if cosines of all angles are small
        // (all angles are ~90 degree) then write quandrange
        // vertices to resultant sequence
        // H9_INFO("area = %f, aspect_ratio = %f, max_cosine = %f, area_ratio = %f", 
        //     area, (float)r.width / (float)r.height, max_cosine, area_ratio);
        if (area_ratio < m_area_ratio) {
            H9_WARN("cannot find QRCode, due to area_ratio=%f", area_ratio);
            continue;
        }
        if (max_cosine < m_max_cosine) {
            vertices.assign(approx.begin(), approx.end());
            return true;
        }
        H9_WARN("cannot find QRCode, due to max_cosine=%f", max_cosine);
    }
    return false;
}

bool QRCodeFinder::find_bymark(std::vector<cv::Point2f> &centers)
{
    H9_TIMER("QRCodeFinder::find_bymark()");
    double minVal, maxVal;
    cv::minMaxLoc(m_src, &minVal, &maxVal);
    cv::Mat bin;
    cv::threshold(m_src, bin, 0.618 * (minVal + maxVal), 255, cv::THRESH_BINARY_INV);
    cv::adaptiveThreshold(m_src, m_src, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY_INV, m_code_unit_width, 6);
    // cv::threshold(m_src, m_src, 0.5 * (minVal + maxVal), 255, cv::THRESH_BINARY_INV && cv::THRESH_OTSU);
    if (!m_debug_level) {
        cv::imwrite(m_base_name + "-qrcode-bin.png", m_src);
    }
    cv::medianBlur(m_src, m_src, 3);
    cv::bitwise_and(m_src, bin, m_src);
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, m_close_sz);

    const cv::Mat close_ker = cv::getStructuringElement(cv::MORPH_RECT, m_close_sz);
    const cv::Mat open_ker = cv::getStructuringElement(cv::MORPH_RECT, m_open_sz);
    cv::morphologyEx(m_src, m_src, cv::MORPH_CLOSE, close_ker);
    cv::morphologyEx(m_src, m_src, cv::MORPH_OPEN, open_ker);
    if (!m_debug_level) {
        cv::imwrite(m_base_name + "-qrcode-morph.png", m_src);
    }    
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hiers;
    cv::findContours(m_src, contours, hiers, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    if (contours.empty())
        return false;
    vector<int> idxs;
    size_t ncontours = contours.size();
    cv::Mat canvas;
    if (!m_debug_level)
        cv::cvtColor(m_src, canvas, cv::COLOR_GRAY2BGR);
    auto hasChild = [](const vector<cv::Vec4i>& hier, int idx) {
        return hier[idx][2] >= 0;
    };
    auto noParent = [](const vector<cv::Vec4i>& hier, int idx) {
        return hier[idx][3] < 0;
    };
    for (size_t i = 0; i < ncontours; i++)
    {
        if (hasChild(hiers, i) && noParent(hiers, i)) {
            int son_idx = hiers[i][2];
            if (hasChild(hiers, son_idx)) {
                int grandson_idx = hiers[son_idx][2];
                if (isSquare(contours[i]) && isSquare(contours[son_idx]) && isSquare(contours[grandson_idx])) {
                    double area = cv::contourArea(contours[grandson_idx]);
                    if (area < m_code_estimated_area * 0.5 || m_code_estimated_area * 2 < area) 
                        continue;
                    if (!m_debug_level) {
                        cv::drawContours(canvas, contours, i, cv::Scalar(0, 0, 255), 2);
                        cv::drawContours(canvas, contours, son_idx, cv::Scalar(0, 255, 255), 2);
                        cv::drawContours(canvas, contours, grandson_idx, cv::Scalar(0, 255, 0), 2);
                    }
                    m_contours.push_back(contours[i]);
                    m_contours.push_back(contours[son_idx]);
                    m_contours.push_back(contours[grandson_idx]);
                    idxs.push_back(i);
                }
            }
        }
    }
    if (!m_debug_level)
        cv::imwrite(m_base_name + "-qrcode-res.png", canvas);  
    centers.resize(3);
    if (idxs.size() > 3) 
        return false;
    else if (idxs.size() == 3) {
        for (size_t i = 0; i < 3; i++) {
            cv::Moments mu = cv::moments(contours[idxs[i]]);
            centers[i].x = mu.m10 / mu.m00;
            centers[i].y = mu.m01 / mu.m00;
        }
        double min_dist_sum = numeric_limits<double>::max();
        int vertex_idx = -1;
        for (size_t i = 0; i < 3; i++) {
            double dist_sum = dist_sq(centers[i], centers[(i+1)%3]) + dist_sq(centers[i], centers[(i+2)%3]);
            if (dist_sum < min_dist_sum) {
                min_dist_sum = dist_sum;
                vertex_idx = i;
            }
        }
        if (vertex_idx != 1)
            std::swap(centers[vertex_idx], centers[1]);
        
        // auto bottom_one = std::max_element(centers.begin(), centers.end(), [](cv::Point2f lhs, cv::Point2f rhs){return lhs.y < rhs.y;});
        // std::swap(centers[0], *bottom_one);
        // auto right_one = std::max_element(centers.begin(), centers.end(), [](cv::Point2f lhs, cv::Point2f rhs){return lhs.x < rhs.x;});
        // std::swap(centers[2], *right_one);
        if (!m_debug_level) {
            cv::Point2f qrcode_center = (centers[0] + centers[2]) * 0.5;
            for (size_t i = 0; i < 3; i++)  
                cv::line(canvas, centers[i], centers[(i+1)%3], cv::Scalar(255,0,255), 1, cv::LINE_AA);
            cv::circle(canvas, qrcode_center, 5, cv::Scalar(255,0,0), 2, cv::LINE_AA);
        }
    } else if (idxs.size() == 2) {
        cv::Moments mu;
        mu = cv::moments(contours[idxs[0]]);
        cv::Point2f center0(mu.m10 / mu.m00, mu.m01 / mu.m00);
        mu = cv::moments(contours[idxs[1]]);
        cv::Point2f center1(mu.m10 / mu.m00, mu.m01 / mu.m00);
        float dx = abs(center1.x - center0.x);
        float dy = abs(center1.y - center0.y);
        float threshold = 2;
        float ratio = dx / dy < 1 ? dy / dx : dx / dy;
        if (ratio < threshold) { // diagonal
            centers[0].x = min(center0.x, center1.x);
            centers[0].y = max(center0.y, center1.y);
            centers[1].x = min(center0.x, center1.x);
            centers[1].y = min(center0.y, center1.y);
            centers[2].x = max(center0.x, center1.x);
            centers[2].y = min(center0.y, center1.y);
        } else {
            if (dx < dy) {
                centers[0] = center0.y > center1.y ? center0 : center1;
                centers[1] = center0.y > center1.y ? center1 : center0;
                centers[2].x = centers[0].y - centers[1].y + centers[1].x;
                centers[2].y = centers[1].x - centers[0].x + centers[1].y;
            } else {
                centers[1] = center0.x < center1.x ? center0 : center1;
                centers[2] = center0.x < center1.x ? center1 : center0;
                centers[0].x = centers[1].y - centers[2].y + centers[1].x;
                centers[0].y = centers[2].x - centers[1].x + centers[1].y;
            }
        }
    } else 
        return false;

    if (!m_debug_level)
        cv::imwrite(m_base_name + "-qrcode-res.png", canvas);  
    return true; 
}

/**
 * @description: finds a cosine of angle between vectors from pt0->pt1 and from pt0->pt2
 * @param {Point const} &pt1
 * @param {Point const} &pt2
 * @param {Point const} &pt0
 * @return {dobule} cosine of angle
 */
double QRCodeFinder::angle(Point const &pt1, Point const &pt2, Point const &pt0)
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1 * dy2) / sqrt((dx1*dx1 + dy1 * dy1)*(dx2*dx2 + dy2 * dy2) + 1e-10);
}

bool MSERFinder::find(cv::Mat const & img)
{
    if (img.type() != CV_8UC1) return false;
    double thresh;
    int previous_count = 0;
    int current_count = 0;
    for (thresh = m_start_value; thresh < m_end_value; thresh += m_step) {
        cv::threshold(img, m_binary, thresh, 255, cv::THRESH_BINARY);
        current_count = cv::countNonZero(m_binary);
        if (current_count == 0) continue;
        int increment = abs(current_count - previous_count);
        if (increment < m_min_increment) {
            m_min_increment = increment;
            m_res_thresh = thresh;
        }
        previous_count =  current_count;
    }
    return true;
}



}//H9
