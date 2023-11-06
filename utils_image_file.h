#if !defined(UTILS_IMAGE_FILE_H)
#define UTILS_IMAGE_FILE_H

#pragma once
#include <list>
#include "Levenshtein.h"
#include <opencv2/core.hpp>
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING 1
#include <experimental/filesystem>


/**
		@brief
		//return the ascii caracter that corresponds to index class output by the dnn
			@param classe : integer index = class identifier, output by the object detection dnn
			@return an ascii caracter
			@see
			*/
char get_char(const int classe);
/**
		@brief
		//checks if the characters contained in lpn are compatible with the alphabet
			@param lpn: the registration of the vehicle as a string
			@return
			@see
			*/
bool could_be_lpn(const std::string& lpn);
/**
		@brief
		returns the true license plate number out of a filename
		you must place the true license plate number in the image filename this way : number+underscore+license plate number,
		for instance filename 0000000001_3065WWA34.jpg will be interpreted as an image with the license plate 3065WWA34 in it.
			@param filename: the image filename that contains in it the true registration number
			@return the lpn contained in the image filename
			@see
			*/
std::string getTrueLPN(const std::string& filename, const bool& vrai_lpn_after_underscore);
			
//extracts from a test directory all images files 
void load_images_filenames(const std::string& dir, std::list<std::string>& image_filenames);

void show_boxes(const cv::Mat& frame, const std::list<cv::Rect>& true_boxes, const std::list<int>& classesId);
void drawPred(const int classId, const int left, const int top, const int right, const int bottom, cv::Mat& frame, const std::vector<std::string>& classes);
#endif // !defined(UTILS_IMAGE_FILE_H)
