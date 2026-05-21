# JPEG Compression in OpenCV

## Overview

This is an implementation of the JPEG compression algorithm in OpenCV, using Discrete Cosine Transform (DCT) and without Huffman encoding, the output being a custom `.bin` file.

## Binary file structure

```
rows
cols
rows_with_padding
cols_with_padding

for each channel: (3 channels)
    nr_blocks
    for each block:
        nr_pairs
        for each pair:
            zeros
            value
```

## Implementation

This implementation works for **BGR** uncompressed images.

### Steps:

1. **Color Space Conversion**: Convert the image from BGR to YCrCb color space.
2. **Channel Splitting**: The image is split into 3 channels (Y, Cr, Cb).
3. **Block Splitting**: Divide each channel into 8×8 blocks.
4. **Discrete Cosine Transform (DCT)**: Transform each 8x8 block from spatial domain into frequency domain.
5. **Quantization**: Reduce most of the less important high frequency DCT coefficients to zero.
6. **Zigzag Scanning**: Arrange the coefficients into a 1D array, grouping likely zero values together.
7. **Run Length Encoding**: Compress the zigzag sequence by storing pairs containing the number of consecutive zeros followed by the next non-zero value.

## How to use it

- After importing the library, initialize the object with the constructor `JPEGCompressor compressor`.
- The `compress(path)` method can be used to compress an image, and it automatically saves it as `img.bin`.
- The `decompress(path)` will return a `Mat_<Vec3b>` decompressed image from the binary file given in the path argument.

## Results

The implemented algorithm was able to achieve a compression ratio of **~3:1** for different images.