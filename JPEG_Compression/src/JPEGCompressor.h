#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>

using namespace cv;

class JPEGCompressor
{
public:
	JPEGCompressor(Mat_<Vec3b> initial_img);
	void compress();
	void save_as_binary(std::string path);

private:
	void split_Y_Cr_Cb(Mat_<Vec3b> initial_img);
	void image_padding();
	void process_blocks_forward();
	Mat_<float> f_dct(const Mat_<float>& block_8x8_float);
	Mat_<int> quantization(const Mat_<float>& res, int channel);
	std::vector<int> zig_zag(const Mat_<int>& quantized_block);

	std::vector<Mat_<uchar>> channels;
	std::vector<Mat_<uchar>> padded_channels;
	int rows, rows_with_padding;
	int cols, cols_with_padding;
	std::vector<std::vector<int>> quantization_table_luminance;
	std::vector<std::vector<int>> quantization_table_chrominance;
	std::vector<std::pair<int, int>> zig_zag_table;

	enum {
		Y_CHANNEL,
		CR_CHANNEL,
		CB_CHANNEL
	};

	struct RLEpair 
	{
		int zeros;
		int value;
	};

	std::vector<RLEpair> run_length_encoding(const std::vector<int>& zig_zagged);
	std::vector<std::vector<std::vector<RLEpair>>> compressed_img; //channels, blocks, pairs
};

