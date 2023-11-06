#if !defined(UTILS_ANPR_ONNX_H)
#define UTILS_ANPR_ONNX_H

#pragma once


#include <opencv2/core.hpp>
//#include "Levenshtein.h"
#include "Line.h"

enum Det {
    tl_x = 0,
    tl_y = 1,
    br_x = 2,
    br_y = 3,
    score = 4,
    class_idx = 5
};

struct Detection {
    cv::Rect bbox;
    float score;
    int class_idx;
};


void sort_from_left_to_right(std::list<cv::Rect>& boxes, std::list<float>& confidences, std::list<int>& classIds);

cv::Rect get_inter(const cv::Rect& r1, const cv::Rect& r2);

bool is_in_rect_if(const cv::Rect& box, const cv::Rect& rect_im);

int get_median_height(const std::list<cv::Rect>& boxes);

float iou(const cv::Rect& r1, const cv::Rect& r2);

void filter_iou2(
	std::list<cv::Rect>& boxes,
	std::list<float>& confidences,
	std::list<int>& classIds, float nmsThreshold);

void filter_iou(std::vector<int>& classIds,
	std::vector<float>& confidences,
	std::vector<cv::Rect>& vect_of_detected_boxes, const float& nmsThreshold
);

void get_barycenters(const std::list<cv::Rect>& boxes, std::list<cv::Point2f>& les_points);

#ifdef _DEBUG
			//cette fonction verifie que la liste est trie de gauche a droite 
bool debug_left(const std::list<cv::Rect>& boxes);
#endif //_DEBUG

void filtre_grubbs_sides(const std::list<cv::Rect>& boxes, std::list<float>& angles_with_horizontal_line,
	float& mean_angles_par_rapport_horiz,
	float& standard_deviation_consecutives_angles,
	std::list<int>& interdistances,
	float& mean_interdistances,
	float& standard_deviation_interdistances,
	float& mean_produit_interdistance_avec_angle, float& standard_deviation_produit_interdistance_avec_angle);

void is_bi_level_plate(const std::list<cv::Rect>& boxes, const std::list<float>& l_confidences, const std::list<int>& l_classIds,
	std::list<cv::Rect>& l_reordered, std::list<float>& l_reordered_confidences, std::list<int>& l_reordered_classIds, std::list<int>& levels);

int is_digit(const char input);

std::string get_lpn(
	const std::vector<cv::Rect>& vect_of_detected_boxes,
	const std::vector<float>& confidences, const std::vector<int>& classIds,
	std::vector<cv::Rect>& tri_left_vect_of_detected_boxes,
	std::vector<float>& tri_left_confidences, std::vector<int>& tri_left_classIds, float nmsThreshold);



/***
	 * @brief Padded resize
	 * @param src - input image
	 * @param dst - output image
	 * @param out_size - desired output size
	 * @return padding information - pad width, pad height and zoom scale
	 */
std::vector<float> LetterboxImage(const cv::Mat & src, cv::Mat & dst, const cv::Size & out_size = cv::Size(640, 640)); 
/***
  * @brief Rescale coordinates to original input image
  * @param data - detection result after inference and nms
  * @param pad_w - width padding
  * @param pad_h - height padding
  * @param scale - zoom scale
  * @param img_shape - original input image shape
  */

void ScaleCoordinates(std::vector<Detection>& data, float pad_w, float pad_h,
							   float scale, const cv::Size& img_shape);


std::vector<std::vector<Detection>> PostProcessing(
	
	float* output, // output of onnx runtime ->>> 1,25200,85
	size_t dimensionsCount,
	size_t size, // 1x25200x85=2142000
	int dimensions,
	float modelWidth, float modelHeight, const cv::Size& img_shape,
	float conf_threshold, float iou_threshold);


std::vector<std::vector<Detection>> PostProcessing(
	float* output, // output of onnx runtime ->>> 1,25200,85
	size_t dimensionsCount,
	size_t size, // 1x25200x85=2142000
	int dimensions,
	//pad_w is the left (and also right) border width in the square image feeded to the model
	float pad_w, float pad_h, float scale, const cv::Size& img_shape,
	float conf_threshold, float iou_threshold);
#endif // !defined(UTILS_ANPR_ONNX_H)
