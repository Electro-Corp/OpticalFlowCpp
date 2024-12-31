// OpticalFlow.cpp : Defines the entry point for the application.
//

#include "OpticalFlow.h"
#include "Frame.h"

int sobelX[3][3] = {
	{-1, 0 , 1},
	{-2, 0, 2},
	{-1, 0, 1}
};

int sobelY[3][3] = {
	{-1, -2 , -1},
	{0, 0, 0},
	{1, 2, 1}
};

// CCIR 601 wegihts
double rIntensityConstant = 0.2989;
double gIntensityConstant = 0.5870;
double bIntensityConstant = 0.1140;


typedef struct {
	double sX, sY;
} sobelOut;

int main()
{
	printf("CT's Optical Flow Tool v0.1\n");
	Frame frame1("1.png");
	Frame frame2("2.png");

	printf("FRAME 1(%d, %d) | FRAME 2(%d, %d)\n", frame1.getWidth(), frame1.getHeight(), frame2.getWidth(), frame2.getHeight());

	int width = frame1.getWidth();
	int height = frame2.getHeight();

	Frame temporal("temporal.png", width, height);
	Frame spatial("spatial.png", width, height);
	Frame f1Gray("f1gray.png", width, height);

	assert(frame1.getHeight() == frame2.getHeight());
	assert(frame1.getWidth() == frame2.getWidth());

	// Preform Lucas-Kande Optical Flow

	printf("Begin Temporal Gradient calculation\n");
	// Get temporal gradient (just the brightness lmao)
	for (int y = 0; y < height - 1; y++) {
		PixelRow f1Row = *(frame1.getRow(y));
		PixelRow f2Row = *(frame2.getRow(y));
		PixelRow temRow = *(temporal.getRow(y));
		PixelRow grayRow = *(f1Gray.getRow(y));
		for (int x = 0; x < f1Row.getSize() - 1; x++) {
			// Calculate first brigness
			Pixel p1 = f1Row[x];
			double oneIntensity = (p1.r * rIntensityConstant) + (p1.g * gIntensityConstant) + (p1.b * bIntensityConstant);
			Pixel g = {oneIntensity, oneIntensity, oneIntensity};
			grayRow.setPixel(g, x);

			// Calculate second brigness
			Pixel p2 = f2Row[x];
			double twoIntensity = (p2.r * rIntensityConstant) + (p2.g * gIntensityConstant) + (p2.b * bIntensityConstant);

			// Calculate difference
			double dI = oneIntensity - twoIntensity;

			Pixel tmp = { dI, dI, dI };
			temRow.setPixel(tmp, x);
		}
		printf("%f percent complete\r", (((double)y / (double)(height - 1)) * 100.0));
		temporal.setRow(temRow, y);
		f1Gray.setRow(grayRow, y);
	}

	temporal.Write();
	f1Gray.Write();

	printf("Begin Spatial Gradient calculation\n");

	// Cheap hack cuz im too lazy to add a special edge case for the edges
	Frame hack("hack.png", width + 2, height + 2);

	// Bascially pad the image
	for (int y = 1; y < height; y++) {
		PixelRow grayRow = *(f1Gray.getRow(y));
		PixelRow hackRow = *(hack.getRow(y));
		for (int x = 1; x < width; x++) {
			hackRow.setPixel(grayRow[x - 1], x);
		}
		hack.setRow(hackRow, y);
	}

	hack.Write();

	Frame sobel("sobel.png", width, height);

	for (int y = 1; y < height - 2; y++) {
		PixelRow grayRow = *(hack.getRow(y));
		PixelRow grayRowPrev = *(hack.getRow(y - 1));
		PixelRow grayRowAfter = *(hack.getRow(y + 1));
		PixelRow sobelRow = *(sobel.getRow(y - 1));
		//PixelRow f2Row = *(frame2.getRow(y));
		for (int x = 1; x < grayRow.getSize() - 2; x++) {
			// Preform convlution on the first frame
			// this is so ugly but i wrote it in 10 minutes 
			// pls dont judge >///<

			// SobelX

			double accX = 0;

			accX += grayRowPrev[x - 1].r * sobelX[0][0];
			accX += grayRowPrev[x].r * sobelX[0][1];
			accX += grayRowPrev[x + 1].r * sobelX[0][2];

			accX += grayRow[x - 1].r * sobelX[1][0];
			accX += grayRow[x].r * sobelX[1][1];
			accX += grayRow[x + 1].r * sobelX[1][2];

			accX += grayRowAfter[x - 1].r * sobelX[2][0];
			accX += grayRowAfter[x].r * sobelX[2][1];
			accX += grayRowAfter[x + 1].r * sobelX[2][2];

			double accY = 0;

			accY += grayRowPrev[x - 1].r * sobelY[0][0];
			accY += grayRowPrev[x].r * sobelY[0][1];
			accY += grayRowPrev[x + 1].r * sobelY[0][2];

			accY += grayRow[x - 1].r * sobelY[1][0];
			accY += grayRow[x].r * sobelY[1][1];
			accY += grayRow[x + 1].r * sobelY[1][2];

			accY += grayRowAfter[x - 1].r * sobelY[2][0];
			accY += grayRowAfter[x].r * sobelY[2][1];
			accY += grayRowAfter[x + 1].r * sobelY[2][2];


			Pixel sobelTmp = { accX, accY, 0 };
			sobelRow.setPixel(sobelTmp, x - 1);
		}
		printf("%f percent complete\r", (((double)y / (double)(height - 2)) * 100.0));
		sobel.setRow(sobelRow, y - 1);
	}

	sobel.Write();

	printf("Flipping Sobel to create AT matrix\n");

	Frame sobelT("sobelT.png", height, width);

	for (int y = 0; y < width; y++) {
		PixelRow sobelTRow = *(sobelT.getRow(y));
		for (int x = 0; x < height; x++) {
			Pixel newGuy = (*sobel.getRow(x))[y];
			sobelTRow.setPixel(newGuy, x);
		}
		sobelT.setRow(sobelTRow, y);
	}
	sobelT.Write();




	return 0;
}
