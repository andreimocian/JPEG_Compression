#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;

class JPEGCompressor
{
public:
	JPEGCompressor(Mat_<Vec3b> initial_img);
	void compress();

private:
	void split_Y_Cr_Cb(Mat_<Vec3b> initial_img);
	void image_padding();
	void process_blocks();
	Mat_<float> f_dct(const Mat_<float>& block_8x8_float);
	Mat_<int> quantization(const Mat_<float>& res, int channel);

	std::vector<Mat_<uchar>> channels;
	std::vector<Mat_<uchar>> padded_channels;
	int rows, rows_with_padding;
	int cols, cols_with_padding;
	std::vector<std::vector<int>> quantization_table_luminance;
	std::vector<std::vector<int>> quantization_table_chrominance;

	enum {
		Y_CHANNEL,
		CR_CHANNEL,
		CB_CHANNEL
	};
};

