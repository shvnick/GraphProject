#include <fstream>
#include <cmath>
#include <cstdint>
#include <vector>
#include <iostream>

// z = sinc(function(x,y))

#pragma pack(push, 1)
struct TGA_Header
{
	uint8_t id_len;			// Длина идентификатора
	uint8_t pal_type;		// Тип палитры
	uint8_t img_type;		// Тип изображения
	uint8_t pal_desc[5];	// Описание палитры
	uint16_t x_pos;			// Положение по оси X
	uint16_t y_pos;			// Положение по оси Y
	uint16_t width;			// Ширина
	uint16_t height;		// Высота
	uint8_t depth;			// Глубина цвета
	uint8_t img_desc;		//Описатель изображения
};
#pragma pack(pop)

constexpr uint16_t IMG_WIDTH = 1920;
constexpr uint16_t IMG_HEIGHT = 1080;
constexpr uint32_t COL_BACKGROUND = 0xff033f33;
constexpr uint32_t COL_FOREGROUND = 0xffcfcfcf;

double sinc(double x){
	if (x==0) return 1.;
	return sin(x) / x;
}

double function(double x, double y){
	return sinc(hypot(x, y));
}

void dek2scr(double x, double y, double z,
			 double &sx, double &sy)
{
	double koefx = 11., koefy = 11., koefz = 70.;

	sx = int(IMG_WIDTH/2 - koefx*x * cos(M_PI/6) + koefy*y * cos(M_PI/6));
	sy = int(IMG_HEIGHT/2 + koefx*x * sin(M_PI/6) + koefy*y * sin(M_PI/6) - koefz*z);
}

int main()
{
	TGA_Header hdr {};
	hdr.width = IMG_WIDTH;
	hdr.height = IMG_HEIGHT;
	hdr.depth = 32;
	hdr.img_type = 2;
	hdr.img_desc = 0x28;

	std::vector<uint32_t> picture(IMG_WIDTH * IMG_HEIGHT);
	for (auto && p : picture) p = COL_BACKGROUND;

	double z, sx, sy;
	double gap1 = 0.5, gap2 = 0.01;

	std::vector<int> HorizonLine(IMG_WIDTH);
	for (auto&& H : HorizonLine) H = IMG_HEIGHT;

	for (double x = 50.; x >= -50.; x -= gap1) {
			for (double y = 50.; y >= -50.; y -= gap2) {
				z = function(x,y);
				dek2scr(x,y,z,sx,sy);

				if (sx < 0 || sx >= IMG_WIDTH || sy > HorizonLine[sx] || sy < 0) continue;
				HorizonLine[sx]=sy;
				picture[sy*IMG_WIDTH + sx] = COL_FOREGROUND;
				}
			}

	for (auto&& H : HorizonLine) H = IMG_HEIGHT;

	for (double x = 50.; x >= -50.; x -= gap2) {
			for (double y = 50.; y >= -50.; y -= gap1) {
				z = function(x,y);
				dek2scr(x,y,z,sx,sy);

				if (sx < 0 || sx >= IMG_WIDTH || sy > HorizonLine[sx] || sy < 0) continue;
				HorizonLine[sx]=sy;
				picture[sy*IMG_WIDTH + sx] = COL_FOREGROUND;
				}
			}

	std::fstream tga_file {"output.tga", std::ios::out|std::ios::binary};;
	tga_file.write(reinterpret_cast<char*>(&hdr),sizeof(TGA_Header));
	tga_file.write(reinterpret_cast<char*>(&picture[0]),IMG_WIDTH*IMG_HEIGHT*4);
	tga_file.close();

}
