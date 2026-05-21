#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <fstream>
#include <filesystem>

using namespace cv;

class JPEGCompressor
{
public:
	JPEGCompressor();
	void compress(std::string path);
	Mat_<Vec3b> decompress(std::string path);

private:
	std::vector<Mat_<uchar>> split_Y_Cr_Cb(Mat_<Vec3b> initial_img);
	void image_padding();
	void process_blocks_forward();
	void read_binary_file(std::string path);
	void save_as_binary(std::string path);
	void load_initial_image(std::string path);
	float calculate_compression_ratio(std::string initial_path, std::string compressed_path);

	/* Compression */

	Mat_<float> f_dct(const Mat_<float>& block_8x8_float);
	Mat_<int> quantization(const Mat_<float>& res, int channel);
	std::vector<int> zig_zag(const Mat_<int>& quantized_block);

	/* Decompression */

	Mat_<int> inverse_zig_zag(const std::vector<int>& zig_zagged);
	Mat_<float> dequantization(const Mat_<int>& quantized_block, int channel);
	Mat_<float> i_dct(const Mat_<float>& d_block);


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
		unsigned char zeros;
		short int value;
	};

	std::vector<RLEpair> run_length_encoding(const std::vector<int>& zig_zagged);
	std::vector<std::vector<std::vector<RLEpair>>> compressed_img; //channels, blocks, pairs


	std::vector<int> inverse_run_length_encoding(const std::vector<RLEpair>& rle_block);
};

