#include "utils_image_file.h"

#include <opencv2/opencv.hpp>
/**
		@brief
		returns the true license plate number out of a filename
		you must place the true license plate number in the image filename this way : number+underscore+license plate number,
		for instance filename 0000000001_3065WWA34.jpg will be interpreted as an image with the license plate 3065WWA34 in it.
			@param filename: the image filename that contains in it the true registration number
			@return the lpn contained in the image filename
			@see
			*/
std::string getTrueLPN(const std::string& filename, const bool& vrai_lpn_after_underscore)
{
	std::string analysing_string = filename;
	if (analysing_string == "")
		return std::string();
	char sep_underscore = '_';
	size_t index = 0;
	index = (analysing_string.find(sep_underscore));
	if (index != -1)
	{
		std::string subanalysing_string;//la sous chaine
		if (!vrai_lpn_after_underscore) {
			subanalysing_string = analysing_string.substr(0, index);//la sous chaine
		}
		else {
			subanalysing_string = analysing_string.substr(index + 1, analysing_string.length() - (index + 1));//la sous chaine
		}
		if (could_be_lpn(subanalysing_string))
			return subanalysing_string;
		else return std::string();
	}
	else {
		if (could_be_lpn(filename))
			return filename;
		else return std::string();
	}
}

/**
		@brief
		//checks if the characters contained in lpn are compatible with the alphabet
			@param lpn: the registration of the vehicle as a string
			@return
			@see
			*/
bool could_be_lpn(const std::string& lpn) {
	const int number_of_caracters = 36;
	char LATIN_LETTERS_NO_I_O_LATIN_DIGITS[number_of_caracters] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J','K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9','0' };
	std::string::const_iterator it(lpn.begin());
	std::list<char> chars;
	while (it != lpn.end()) {
		int i;
		for (i = 0; i < number_of_caracters; i++) {
			if (*it == LATIN_LETTERS_NO_I_O_LATIN_DIGITS[i]) break;
		}
		if (i < number_of_caracters) {
			it++;
		}
		else return false;
	}
	return true;
}
//extracts from a test directory all images files 
void load_images_filenames(const std::string& dir, std::list<std::string>& image_filenames)
{
	std::experimental::filesystem::path p(dir);
	std::vector<std::experimental::filesystem::directory_entry> v; // To save the file names in a vector.
	if (is_directory(p))
	{
		const std::string dir_path = p.string();
		std::experimental::filesystem::directory_iterator b(p), e;
		for (auto i = b; i != e; ++i)
		{
			if (std::experimental::filesystem::is_regular_file(*i)) {
				std::experimental::filesystem::path fe = i->path().extension();
				std::string extension = fe.string();
				if (extension == ".bmp" || extension == ".BMP" || extension == ".jpg" || extension == ".JPG" || extension == ".jpeg")
				{
					std::experimental::filesystem::path p_(i->path());
					//if you want to select images that have the true license plate number in the image filename
					const bool select_images_with_lpn = true;
					if (select_images_with_lpn) {
						bool vrai_lpn_after_underscore = true;

						//returns the true license plate number out of a filename
							//you must place the true license plate number in the image filename this way : number + underscore + license plate number,
							//for instance filename 0000000001_3065WWA34.jpg will be interpreted as an image with the license plate 3065WWA34 in it.
						std::string ExactLPN(getTrueLPN(p_.stem().string(), vrai_lpn_after_underscore));
						if (ExactLPN.size() > 3 && ExactLPN.size() < 11) {
							image_filenames.push_back(i->path().string());
						}
					}
					else {//take all images files -- output stats impossible
						image_filenames.push_back(i->path().string());
					}
				}
			}
		}
	}
}

/**
		@brief
		//return the ascii caracter that corresponds to index class output by the dnn
			@param classe : integer index = class identifier, output by the object detection dnn
			@return an ascii caracter
			@see
			*/
char get_char(const int classe) {
	const int number_of_caracters = 36;
	char LATIN_LETTERS_NO_I_O_LATIN_DIGITS[number_of_caracters] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J','K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
		'Y', 'Z', '1', '2', '3', '4', '5', '6', '7', '8', '9','0' };
	if (classe >= 0 && classe < number_of_caracters - 1)
		return LATIN_LETTERS_NO_I_O_LATIN_DIGITS[classe];
	else return '?';
}

//this func saves an image composed of input im with a list of roi (colors in green) 
void show_boxes(const cv::Mat& frame, const std::list<cv::Rect>& true_boxes, const std::list<int>& classesId) {
	std::vector<std::string> classes;
	for (size_t i = 0; i < 35; i++) {
		std::string s; s += get_char(int(i));
		classes.push_back(s);
	}
	classes.push_back("[]");
#ifdef _DEBUG
	cv::Scalar mean_ = cv::mean(frame);
	assert(mean_[0] > 30.0f || mean_[0] < 220.0f);
#endif //_DEBUG
	std::string lpn;
	cv::Mat frame_copy = frame.clone();

	cv::Size normalsize(frame.cols * 2, frame.rows * 2);
	cv::resize(frame, frame_copy, normalsize);
	std::list<cv::Rect>::const_iterator it_boxes(true_boxes.begin());
	std::list<int>::const_iterator it_classesId(classesId.begin());
	const cv::Scalar& col = cv::Scalar(0, 255, 0);
	while (it_boxes != true_boxes.end() && it_classesId != classesId.end()) {
		cv::Rect r(it_boxes->x * 2, it_boxes->y * 2, (it_boxes->width) * 2, (it_boxes->height) * 2);
		//::drawPred(r, frame_copy, col);
		::drawPred(*it_classesId, r.x, r.y, r.x + r.width, r.y + r.height, frame_copy, classes);
		lpn += classes[*it_classesId];
		it_boxes++; it_classesId++;
	}
	cv::imwrite("jieguo.jpg", frame_copy);

#ifdef _DEBUG
	bool show_image = true;
	const int time_delay = 4000;
	if (show_image && time_delay >= 0) {
		cv::imshow(lpn, frame_copy);
	}
	if (time_delay >= 0) {
		if (show_image && time_delay == 0) {
			char c = cv::waitKey(time_delay);
		}
		else if (time_delay > 0) {
			char c = cv::waitKey(time_delay);
		}
	}
	if (show_image && time_delay >= 0) {
		cv::destroyAllWindows();
	}
#endif //_DEBUG
}

void drawPred(int classId, int left, int top, int right, int bottom, cv::Mat& frame, const std::vector<std::string>& classes)
{
	cv::rectangle(frame, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(0, 255, 0), 1);
	std::string label;// = cv::format("%.2f", conf);
	if (!classes.empty())
	{
		CV_Assert(classId < (int)classes.size());
		label = classes[classId] //+ ": " + label
			;
	}
	int baseLine;
	cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
	top = cv::max(top, labelSize.height);
	cv::rectangle(frame, cv::Point(left, top - labelSize.height), cv::Point(left + labelSize.width, top + baseLine), cv::Scalar::all(255), cv::FILLED);
	cv::rectangle(frame, cv::Point(left, top - labelSize.height), cv::Point(left + labelSize.width, top + baseLine), cv::Scalar::all(255), cv::FILLED);
	cv::putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar());
}