#include "JPEGCompressor.h"

JPEGCompressor::JPEGCompressor()
{
	this->quantization_table_luminance = {
		{16, 11, 10, 16, 24, 40, 51, 61},
		{12, 12, 14, 19, 26, 58, 60, 55},
		{14, 13, 16, 24, 40, 57, 69, 56},
		{14, 17, 22, 29, 51, 87, 80, 62},
		{18, 22, 37, 56, 68, 109, 103, 77},
		{24, 35, 55, 64, 81, 104, 113, 92},
		{49, 64, 78, 87, 103, 121, 120, 101},
		{72, 92, 95, 98, 112, 100, 103, 99}
	};

	this->quantization_table_chrominance = {
		{17, 18, 24, 47, 99, 99, 99, 99},
		{18, 21, 26, 66, 99, 99, 99, 99},
		{24, 26, 56, 99, 99, 99, 99, 99},
		{47, 66, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99},
		{99, 99, 99, 99, 99, 99, 99, 99}
	};

	this->zig_zag_table = {
		{0,0}, {0,1}, {1,0}, {2,0}, {1,1}, {0,2}, {0,3}, {1,2},
		{2,1}, {3,0}, {4,0}, {3,1}, {2,2}, {1,3}, {0,4}, {0,5},
		{1,4}, {2,3}, {3,2}, {4,1}, {5,0}, {6,0}, {5,1}, {4,2},
		{3,3}, {2,4}, {1,5}, {0,6}, {0,7}, {1,6}, {2,5}, {3,4},
		{4,3}, {5,2}, {6,1}, {7,0}, {7,1}, {6,2}, {5,3}, {4,4},
		{3,5}, {2,6}, {1,7}, {2,7}, {3,6}, {4,5}, {5,4}, {6,3},
		{7,2}, {7,3}, {6,4}, {5,5}, {4,6}, {3,7}, {4,7}, {5,6},
		{6,5}, {7,4}, {7,5}, {6,6}, {5,7}, {6,7}, {7,6}, {7,7}
	};
}

std::vector<Mat_<uchar>> JPEGCompressor::split_Y_Cr_Cb(Mat_<Vec3b> initial_img)
{
	Mat_<uchar> Y(initial_img.rows, initial_img.cols), Cr(initial_img.rows, initial_img.cols), Cb(initial_img.rows, initial_img.cols);
	Vec3b pixel;
	std::vector<Mat_<uchar>> channels;

	for (int i = 0; i < initial_img.rows; i++)
	{
		for (int j = 0; j < initial_img.cols; j++)
		{
			pixel = initial_img(i, j);
			Y(i, j) = pixel[0];
			Cr(i, j) = pixel[1];
			Cb(i, j) = pixel[2];
		}
	}
	channels = { Y, Cr, Cb };
	return channels;
}

void JPEGCompressor::image_padding()
{
	int padded_rows = 0;
	int padded_cols = 0;
	
	for (int i = 0; i < 3; i++)
	{
		padded_rows = this->rows % 8 == 0 ? 0 : (8 - this->rows % 8);
		padded_cols = this->cols % 8 == 0 ? 0 : (8 - this->cols % 8);

		copyMakeBorder(this->channels[i], this->padded_channels[i], 0, padded_rows, 0, padded_cols, BORDER_REPLICATE);
	}
	this->rows_with_padding = this->padded_channels[0].rows;
	this->cols_with_padding = this->padded_channels[0].cols;
}

static Mat_<float> convert_to_float(Mat_<uchar> img)
{
	Mat_<float> intermidiate(img.rows, img.cols);
	for (int i = 0; i < img.rows; i++)
	{
		for (int j = 0; j < img.cols; j++)
		{
			intermidiate(i, j) = (float)img(i, j);
		}
	}
	return intermidiate;
}


/* Compression */


Mat_<float> JPEGCompressor::f_dct(const Mat_<float>& block_8x8_float)
{
	Mat_<float> res(8, 8);
	for (int u = 0; u < 8; u++)
	{
		for (int v = 0; v < 8; v++)
		{
			float Cu = (u == 0 ? 1 / sqrt(2) : 1);
			float Cv = (v == 0 ? 1 / sqrt(2) : 1);

			float sum = 0.0f;
			for (int x = 0; x < 8; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					sum += block_8x8_float(x, y) * cos(((2 * x + 1) * u * CV_PI) / 16) * cos(((2 * y + 1) * v * CV_PI) / 16);
				}
			}
			res(u, v) = (1.0f / 4.0f) * Cu * Cv * sum;
		}
	}

	return res;
}

Mat_<int> JPEGCompressor::quantization(const Mat_<float>& dct_res, int channel)
{
	Mat_<int> res(8, 8);
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (channel == Y_CHANNEL)
				res(i, j) = (int)std::round(dct_res(i, j) / this->quantization_table_luminance[i][j]);
			else
				res(i, j) = (int)std::round(dct_res(i, j) / this->quantization_table_chrominance[i][j]);
		}
	}
	return res;
}

std::vector<int> JPEGCompressor::zig_zag(const Mat_<int>& quantized_block)
{
	std::vector<int> res(64);
	for (int i = 0; i < 64; i++)
	{
		res[i] = quantized_block(this->zig_zag_table[i].first, this->zig_zag_table[i].second);
	}
	return res;
}

std::vector<JPEGCompressor::RLEpair> JPEGCompressor::run_length_encoding(const std::vector<int>& zig_zagged)
{
	std::vector<JPEGCompressor::RLEpair> res;

	unsigned char nr_of_zeroes = 0;

	for (int i = 0; i < 64; i++)
	{
		if (zig_zagged[i] == 0)
		{
			nr_of_zeroes++;
		}
		else
		{
			res.push_back({ nr_of_zeroes, (short int)zig_zagged[i] });
			nr_of_zeroes = 0;
		}
	}
	res.push_back({ 0, 0 });
	return res;
}

void JPEGCompressor::process_blocks_forward()
{
	std::vector<std::vector<std::vector<JPEGCompressor::RLEpair>>> compressed_img(3); //channels, blocks, pairs
	
	for (int channel = 0; channel < 3; channel++)
	{
		for (int i = 0; i < this->rows_with_padding; i += 8)
		{
			for (int j = 0; j < this->cols_with_padding; j += 8)
			{
				Mat_<uchar> block_8x8 = padded_channels[channel](Rect(j, i, 8, 8));
				Mat_<float> block_8x8_float = convert_to_float(block_8x8);
				block_8x8_float -= 128.0f;

				Mat_<float> dct_res = f_dct(block_8x8_float);
				Mat_<int> quantized_block = quantization(dct_res, channel);
				std::vector<int> zig_zagged = zig_zag(quantized_block);
				std::vector<JPEGCompressor::RLEpair> compressed_block = run_length_encoding(zig_zagged);

				compressed_img[channel].push_back(compressed_block);
			}
		}
	}
	this->compressed_img = compressed_img;
}

void JPEGCompressor::save_as_binary(std::string path)
{
	std::ofstream fout(path, std::ios_base::binary);

	fout.write((char*)&this->rows, sizeof(this->rows));
	fout.write((char*)&this->cols, sizeof(this->cols));
	fout.write((char*)&this->rows_with_padding, sizeof(this->rows_with_padding));
	fout.write((char*)&this->cols_with_padding, sizeof(this->cols_with_padding));

	for (int channel = 0; channel < 3; channel++)
	{
		int nr_blocks = this->compressed_img[channel].size();
		fout.write((char*)&nr_blocks, sizeof(nr_blocks));

		for (int block = 0; block < nr_blocks; block++)
		{
			int nr_pairs = this->compressed_img[channel][block].size();
			fout.write((char*)&nr_pairs, sizeof(nr_pairs));

			for (int k = 0; k < nr_pairs; k++)
			{
				JPEGCompressor::RLEpair pair = this->compressed_img[channel][block][k];

				fout.write((char*)&pair.zeros, sizeof(pair.zeros));
				fout.write((char*)&pair.value, sizeof(pair.value));
			}
		}
	}

	fout.close();
}

void JPEGCompressor::load_initial_image(std::string path)
{
	Mat_<Vec3b> initial_img = imread(path, IMREAD_COLOR);
	Mat_<Vec3b> ycrcb_img;

	this->rows = initial_img.rows;
	this->cols = initial_img.cols;
	this->padded_channels.resize(3);
	cvtColor(initial_img, ycrcb_img, COLOR_BGR2YCrCb);

	this->channels = split_Y_Cr_Cb(ycrcb_img);
}

float JPEGCompressor::calculate_compression_ratio(std::string initial_path, std::string compressed_path)
{
	std::uintmax_t initial_size = std::filesystem::file_size(initial_path);
	std::uintmax_t compressed_size = std::filesystem::file_size(compressed_path);

	return (float)initial_size / (float)compressed_size;
}

void JPEGCompressor::compress(std::string path, std::string file_name)
{
	this->load_initial_image(path);
	this->image_padding();
	this->process_blocks_forward();
	this->save_as_binary(file_name);
	std::cout << "Compression ratio: " << this->calculate_compression_ratio(path, file_name) << ":1" << "\n";
}


/* Decompression */


void JPEGCompressor::read_binary_file(std::string path)
{
	std::ifstream fin(path, std::ios_base::binary);

	fin.read((char*)&this->rows, sizeof(this->rows));
	fin.read((char*)&this->cols, sizeof(this->cols));
	fin.read((char*)&this->rows_with_padding, sizeof(this->rows_with_padding));
	fin.read((char*)&this->cols_with_padding, sizeof(this->cols_with_padding));

	this->compressed_img.clear();
	this->compressed_img.resize(3);

	for (int channel = 0; channel < 3; channel++)
	{
		int nr_blocks;
		fin.read((char*)&nr_blocks, sizeof(nr_blocks));

		this->compressed_img[channel].resize(nr_blocks);

		for (int block = 0; block < nr_blocks; block++)
		{
			int nr_pairs;
			fin.read((char*)&nr_pairs, sizeof(nr_pairs));

			this->compressed_img[channel][block].resize(nr_pairs);

			for (int k = 0; k < nr_pairs; k++)
			{
				RLEpair pair;

				fin.read((char*)&pair.zeros, sizeof(pair.zeros));
				fin.read((char*)&pair.value, sizeof(pair.value));

				this->compressed_img[channel][block][k] = pair;
			}
		}
	}

	fin.close();
}

std::vector<int> JPEGCompressor::inverse_run_length_encoding(const std::vector<RLEpair>& rle_block)
{
	std::vector<int> res;

	for (int i = 0; i < rle_block.size(); i++)
	{
		if (rle_block[i].value == 0 && rle_block[i].zeros == 0)
			break;

		for (int j = 0; j < rle_block[i].zeros; j++)
		{
			res.push_back(0);
		}
		res.push_back(rle_block[i].value);
	}

	while (res.size() < 64)
		res.push_back(0);

	return res;
}

Mat_<int> JPEGCompressor::inverse_zig_zag(const std::vector<int>& zig_zagged)
{
	Mat_<int> res(8, 8);

	for (int i = 0; i < 64; i++)
	{
		res(this->zig_zag_table[i].first, this->zig_zag_table[i].second) = zig_zagged[i];
	}

	return res;
}

Mat_<float> JPEGCompressor::dequantization(const Mat_<int>& quantized_block, int channel)
{
	Mat_<float> res(8, 8);
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (channel == Y_CHANNEL)
				res(i, j) = (float)(quantized_block(i, j) * this->quantization_table_luminance[i][j]);
			else
				res(i, j) = (float)(quantized_block(i, j) * this->quantization_table_chrominance[i][j]);
		}
	}
	return res;
}

Mat_<float> JPEGCompressor::i_dct(const Mat_<float>& d_block)
{
	Mat_<float> res(8, 8);
	for (int x = 0; x < 8; x++)
	{
		for (int y = 0; y < 8; y++)
		{
			float sum = 0.0f;

			for (int u = 0; u < 8; u++)
			{
				for (int v = 0; v < 8; v++)
				{
					float Cu = (u == 0 ? 1 / sqrt(2) : 1);
					float Cv = (v == 0 ? 1 / sqrt(2) : 1);

					sum += Cu * Cv * d_block(u, v) * cos(((2 * x + 1) * u * CV_PI) / 16) * cos(((2 * y + 1) * v * CV_PI) / 16);
				}
			}
			res(x, y) = (1.0f / 4.0f) * sum;
		}
	}

	return res;
}

Mat_<Vec3b> JPEGCompressor::decompress(std::string path)
{
	this->read_binary_file(path);

	std::vector<Mat_<uchar>> channels(3);

	for (int channel = 0; channel < 3; channel++)
	{
		channels[channel] = Mat_<uchar>(this->rows_with_padding, this->cols_with_padding);
		int block = 0;

		for (int i = 0; i < this->rows_with_padding; i += 8)
		{
			for (int j = 0; j < this->cols_with_padding; j += 8)
			{
				std::vector<JPEGCompressor::RLEpair> rle_block = this->compressed_img[channel][block];

				std::vector<int> zig_zagged = inverse_run_length_encoding(rle_block);
				Mat_<int> quantized_block = inverse_zig_zag(zig_zagged);
				Mat_<float> dequantizied_block = dequantization(quantized_block, channel);
				Mat_<float> initial_block = i_dct(dequantizied_block);

				initial_block += 128.0f;

				Mat_<uchar> uchar_block;
				initial_block.convertTo(uchar_block, CV_8U);
				uchar_block.copyTo(channels[channel](Rect(j, i, 8, 8)));

				block++;
			}
		}
	}

	Mat_<Vec3b> reconstructed_img_ycrcb;
	merge(channels, reconstructed_img_ycrcb);

	Mat_<Vec3b> reconstructed_img_rgb;
	cvtColor(reconstructed_img_ycrcb, reconstructed_img_rgb, COLOR_YCrCb2BGR);

	Mat_<Vec3b> final_img = reconstructed_img_rgb(Rect(0, 0, this->cols, this->rows)).clone();

	return final_img;
}