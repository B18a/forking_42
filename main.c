#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

typedef char i8;
typedef unsigned char u8;
typedef unsigned short u16;
typedef int i32;
typedef unsigned u32;
typedef unsigned long u64;

#define PRINT_ERROR(cstring) write(STDERR_FILENO, cstring, sizeof(cstring) - 1)

#pragma pack(1)
struct bmp_header
{
	// Note: header
	i8  signature[2]; // should equal to "BM"
	u32 file_size;
	u32 unused_0;
	u32 data_offset;

	// Note: info header
	u32 info_header_size;
	u32 width; // in px
	u32 height; // in px
	u16 number_of_planes; // should be 1
	u16 bit_per_pixel; // 1, 4, 8, 16, 24 or 32
	u32 compression_type; // should be 0
	u32 compressed_image_size; // should be 0
	// Note: there are more stuff there but it is not important here
};

struct file_content
{
	i8*   data;
	u32   size;
};

struct file_content   read_entire_file(char* filename)
{
	char* file_data = 0;
	unsigned long	file_size = 0;
	int input_file_fd = open(filename, O_RDONLY);
	if (input_file_fd >= 0)
	{
		struct stat input_file_stat = {0};
		stat(filename, &input_file_stat);
		file_size = input_file_stat.st_size;
		file_data = mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, input_file_fd, 0);
		close(input_file_fd);
	}
	return (struct file_content){file_data, file_size};
}


void	get_words(int y,int x,u8* pixel_data, u32 row_size,int len)
{
	int j = 0;
	int chars = 0;
	y -= 2;
	x += 2;
	// printf("[%i %i]\n", y, x);

	while (chars < len)
	{
 			u32 pixel_index = (y * row_size) + ((x + j) * 4);
			u8 blue      = pixel_data[pixel_index + 0];
			u8 green     = pixel_data[pixel_index + 1];
			u8 red       = pixel_data[pixel_index + 2];	
            // printf("Pixel (Y:%u,X:%u): B=%u, G=%u, R=%u\n", y, (x+i), blue, green, red);
			if (chars < len)
			{
            	chars++;
				printf("%c", blue);
			}
			if (chars < len)
			{
            	chars++;
				printf("%c", green);
			}
			if (chars < len)
			{
            	chars++;
				printf("%c", red);
			}
		j++;
		if(!(chars % 18))
		{
			j = 0;
			y -= 1;
			
		}
	}
}



int main(int argc, char** argv)
{
	if (argc != 2)
	{
		PRINT_ERROR("Usage: decode <input_filename>\n");
		return 1;
	}
	struct file_content file_content = read_entire_file(argv[1]);
	if (file_content.data == NULL)
	{
		PRINT_ERROR("Failed to read file\n");
		return 1;
	}
	struct bmp_header* header = (struct bmp_header*) file_content.data;
	printf("signature: %.2s\nfile_size: %u\ndata_offset: %u\ninfo_header_size: %u\nwidth: %u\nheight: %u\nplanes: %i\nbit_per_px: %i\ncompression_type: %u\ncompression_size: %u\n", header->signature, header->file_size, header->data_offset, header->info_header_size, header->width, header->height, header->number_of_planes, header->bit_per_pixel, header->compression_type, header->compressed_image_size);



	int count = 0;

	u8	*pixel_data = (u8 *)(file_content.data + header->data_offset);
	for (u32 y = 0; y < header->height; y++) 
	{
	    for (u32 x = 0; x < header->width; x++) 
		{
	        u32 row_size = ((header->width * header->bit_per_pixel + 31) / 32) * 4;

	        u32 pixel_index = (y * row_size) + (x * 4);

			u8 blue      = pixel_data[pixel_index + 0];
			u8 green     = pixel_data[pixel_index + 1];
			u8 red       = pixel_data[pixel_index + 2];

			if(blue == 127 && green == 188 && red == 217)
			{
            	// printf("Pixel (Y:%u,X:%u): B=%u, G=%u, R=%u\n", y, x, blue, green, red);
				count++;
				if(count == 7)
				{
					y++;
					u32 pixel_index2 = (y * row_size) + ((x + 7) * 4);

					u8 blue2      = pixel_data[pixel_index2 + 0];
					// u8 green2     = pixel_data[pixel_index2 + 1];
					u8 red2       = pixel_data[pixel_index2 + 2];
					// printf("[Pixel (Y:%u,X:%u): B=%u, G=%u, R=%u]\n", y, x + 7, blue2, green2, red2);
					// printf("LEN: %i \n",blue2 + red2);
					get_words(y,x,pixel_data, row_size,blue2 + red2 );
				}
			}
	    }
	}
	return 0;
}


// 16	42		-	Yes you found me
// 396	cave	-	Why did the scarec..........?