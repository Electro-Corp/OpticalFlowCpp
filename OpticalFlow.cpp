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

	printf("Begin Spatial Gradient and final Optical Flow calculation\n");

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

			double Amatrix[9][2]{};
			// Compute this with Amatrix for optimization
			double AtMatrix[2][9]{};

			double accX = 0;

			accX += Amatrix[0][0] = AtMatrix[0][0] = grayRowPrev[x - 1].r * sobelX[0][0];
			accX += Amatrix[1][0] = AtMatrix[0][1] = grayRowPrev[x].r * sobelX[0][1];
			accX += Amatrix[2][0] = AtMatrix[0][2] = grayRowPrev[x + 1].r * sobelX[0][2];

			accX += Amatrix[3][0] = AtMatrix[0][3] = grayRow[x - 1].r * sobelX[1][0];
			accX += Amatrix[4][0] = AtMatrix[0][4] = grayRow[x].r * sobelX[1][1];
			accX += Amatrix[5][0] = AtMatrix[0][5] = grayRow[x + 1].r * sobelX[1][2];

			accX += Amatrix[6][0] = AtMatrix[0][6] = grayRowAfter[x - 1].r * sobelX[2][0];
			accX += Amatrix[7][0] = AtMatrix[0][7] = grayRowAfter[x].r * sobelX[2][1];
			accX += Amatrix[8][0] = AtMatrix[0][8] = grayRowAfter[x + 1].r * sobelX[2][2];

			double accY = 0;

			accY += Amatrix[0][1] = AtMatrix[1][0] = grayRowPrev[x - 1].r * sobelY[0][0];
			accY += Amatrix[1][1] = AtMatrix[1][1] = grayRowPrev[x].r * sobelY[0][1];
			accY += Amatrix[2][1] = AtMatrix[1][2] = grayRowPrev[x + 1].r * sobelY[0][2];

			accY += Amatrix[3][1] = AtMatrix[1][3] = grayRow[x - 1].r * sobelY[1][0];
			accY += Amatrix[4][1] = AtMatrix[1][4] = grayRow[x].r * sobelY[1][1];
			accY += Amatrix[5][1] = AtMatrix[1][5] = grayRow[x + 1].r * sobelY[1][2];

			accY += Amatrix[6][1] = AtMatrix[1][6] = grayRowAfter[x - 1].r * sobelY[2][0];
			accY += Amatrix[7][1] = AtMatrix[1][7] = grayRowAfter[x].r * sobelY[2][1];
			accY += Amatrix[8][1] = AtMatrix[1][8] = grayRowAfter[x + 1].r * sobelY[2][2];


			// Calculate (At)(A)
			long double atA[2][2]{};

			for (int i = 0; i < 9; i++) {
				atA[0][0] += Amatrix[i][0] * Amatrix[i][0];
				atA[0][1] += Amatrix[i][0] * Amatrix[i][1];
				atA[1][0] += Amatrix[i][1] * Amatrix[i][0];
				atA[1][1] += Amatrix[i][1] * Amatrix[i][1];
			}

			// Calcualate (At)(temporal)
			long double atB[2][1]{};

			PixelRow temporRow = *(temporal.getRow(y));

			for (int i = 0; i < 9; i++) {
				atB[0][0] += Amatrix[i][0] * temporRow[i].r;
				atB[1][0] += Amatrix[i][1] * temporRow[i].r;
			}

			long double aInv[2][2]{};
			// 

			long double invConst = 1 / ((atA[0][0] * atA[1][1]) - (atA[0][1] * atA[1][0]));

			long double velocityX = 0, velocityY = 0;

			double determinant = (atA[0][0] * atA[1][1]) - (atA[0][1] * atA[1][0]);
			if (std::abs(determinant) < 1e-6) {
				velocityX = velocityY = 0;
			}
			else {
				long double invConst = 1.0 / determinant;
				aInv[0][0] = invConst * atA[1][1];
				aInv[0][1] = -invConst * atA[0][1];
				aInv[1][0] = -invConst * atA[1][0];
				aInv[1][1] = invConst * atA[0][0];

				velocityX = (aInv[0][0] * atB[0][0]) + (aInv[0][1] * atB[1][0]);
				velocityY = (aInv[1][0] * atB[0][0]) + (aInv[1][1] * atB[1][0]);
			}
			

			//printf("%f %f\n%f %f\r", atA[0][0], atA[0][1], atA[1][0], atA[1][1]);

			printf("%f percent complete | VelX %f VelY %f\r", (((double)y / (double)(height - 2)) * 100.0), velocityX, velocityY);

			Pixel sobelTmp = { accX, accY, 0 };
			sobelRow.setPixel(sobelTmp, x - 1);

		}
		
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
