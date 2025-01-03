// OpticalFlow.cpp : Defines the entry point for the application.
//

#include "OpticalFlow.h"
#include "Frame.h"

// CONFIG
int temporalCutoff = 5; // At what pixel difference do we consider it to be.. actually meaningful?

std::string file1 = "1decSmallblur.png";
std::string file2 = "2decSmallblur.png";

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

typedef struct {
	Fl_Box* bufferBox;
	Fl_Box* imageBox;
} TEMPORAL_VIEW_UPDATE;

typedef struct {
	int oneOrTwo;
	Fl_Box* box;
} OPEN_IMAGE_DATA ;


static Fl_PNG_Image* temporalImage = nullptr;
static Fl_PNG_Image* opticalImage = nullptr;

static Fl_PNG_Image* prevImageOne = nullptr;
static Fl_PNG_Image* prevImageTwo = nullptr;

static std::string newStuff = "Temporal Cutoff: 50";

long double scaleToTensPlace(long double num);

void GenerateTemporal(std::string f1, std::string f2);

void OpticalFlow(std::string f1, std::string f2);


void temporalSliderCallBack(Fl_Widget* widget, void* data) {
	Fl_Slider* slider = (Fl_Slider*)widget;
	int value = slider->value();

	temporalCutoff = value;

	TEMPORAL_VIEW_UPDATE* guy = (TEMPORAL_VIEW_UPDATE*)data;

	// Update the label or perform some operation
	Fl_Box* box = (Fl_Box*)guy->bufferBox;
	//char buffer[256];
	//sprintf_s(buffer, "Temporal Cutoff: %d", temporalCutoff);
	newStuff = std::string{ std::string("Temporal Cutoff: ") + std::to_string(temporalCutoff)};
	box->label(newStuff.c_str());

}

void temporalGenerateCallBack(Fl_Widget* widget, void* data) {

	TEMPORAL_VIEW_UPDATE* guy = (TEMPORAL_VIEW_UPDATE*)data;
	Fl_Window* messageBox = new Fl_Window(300, 150, "Wait");
	Fl_Box* messageText = new Fl_Box(20, 40, 260, 30, "Wait for temporal generation...");
	messageText->box(FL_FLAT_BOX);
	messageText->labelsize(14);
	messageText->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	messageBox->add(messageText);

	messageBox->end();
	messageBox->show();

	std::thread temporalThread(GenerateTemporal, file1, file2);
	temporalThread.join();

	messageBox->hide();

	if (temporalImage) delete temporalImage; // Free previous image
	temporalImage = new Fl_PNG_Image("temporal.png");
	Fl_Box* imageBox = (Fl_Box*)guy->imageBox;
	Fl_Image* scaled_image = temporalImage->copy(270, 180);
	imageBox->image(scaled_image);
	imageBox->redraw();
}

void opticalGenerateCallBack(Fl_Widget* widget, void* data) {
	Fl_Window* messageBox = new Fl_Window(300, 150, "Wait for Lucas-Kande optical flow...");
	Fl_Box* messageText = new Fl_Box(20, 40, 260, 30, "Wait for Optical Flow...");
	messageText->box(FL_FLAT_BOX);
	messageText->labelsize(14);
	messageText->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
	messageBox->add(messageText);

	messageBox->end();
	messageBox->show();

	std::thread opticalThread(OpticalFlow, file1, file2);
	opticalThread.join();

	messageBox->hide();

	if (opticalImage) delete opticalImage; // Free previous image
	opticalImage = new Fl_PNG_Image("1WithVelocites.png");
	Fl_Box* imageBox = (Fl_Box*)data;
	Fl_Image* scaled_image = opticalImage->copy(270, 180);
	imageBox->image(scaled_image);
	imageBox->redraw();
}

void openFileCallback(Fl_Widget* widget, void* data) {
	Fl_File_Chooser chooser(".", "Image Files (*.{png,jpg,jpeg,bmp,gif})", Fl_File_Chooser::SINGLE, "Choose an Image File");
	chooser.show();

	while (chooser.shown()) {
		Fl::wait();
	}

	if (chooser.value() != nullptr) {
		OPEN_IMAGE_DATA* oid = (OPEN_IMAGE_DATA*)data;
		std::string filePath = chooser.value();
		if (oid->oneOrTwo == 1)
			file1 = filePath;
		else
			file2 = filePath;
		Fl_Box* f1 = (Fl_Box*)oid->box;
		if (prevImageOne) delete prevImageOne;
		prevImageOne = new Fl_PNG_Image(filePath.c_str());
		Fl_Image* scaled_image = prevImageOne->copy(270, 180);
		f1->image(scaled_image);
		f1->redraw();
	}
}



// UI offset
int temporalYOffset = 240;


int main()
{
	printf("CT's Optical Flow Tool v0.1\n");
	Fl_Window* window = new Fl_Window(600, 550, "CT Optical Flow");

	
	// Original images
	Fl_PNG_Image ogImage1(file1.c_str());
	Fl_Box* imageBoxOg1 = new Fl_Box(0, 50, 270, 180);
	Fl_Image* scaledOG1 = ogImage1.copy(270, 180);
	imageBoxOg1->image(scaledOG1);
	Fl_PNG_Image ogImage2(file2.c_str());
	Fl_Box* imageBoxOg2 = new Fl_Box(300, 50, 270, 180);
	Fl_Image* scaledOG2 = ogImage2.copy(270, 180);
	imageBoxOg2->image(scaledOG2);


	Fl_Button* openImage1Button = new Fl_Button(0, 0, 150, 40, "Open Image 1");

	OPEN_IMAGE_DATA but1Dat = { 1, imageBoxOg1 };
	openImage1Button->callback(openFileCallback, &but1Dat);

	Fl_Button* openImage2Button = new Fl_Button(300, 0, 150, 40, "Open Image 2");

	OPEN_IMAGE_DATA but2Dat = { 2, imageBoxOg2 };
	openImage2Button->callback(openFileCallback, &but2Dat);


	GenerateTemporal("1decBlur.png", "2decBlur.png");


	// Temporal Preview
	Fl_PNG_Image temporalImage("temporal.png");
	Fl_Box* imageBox = new Fl_Box(0, 0 + temporalYOffset, 270, 180);
	Fl_Image* scaled_image = temporalImage.copy(270, 180);
	imageBox->image(scaled_image);

	Fl_Slider* temporalCutoffSlider = new Fl_Slider(0, 200 + temporalYOffset, 300, 20);
	temporalCutoffSlider->type(FL_HOR_NICE_SLIDER);
	temporalCutoffSlider->bounds(0, 255);
	temporalCutoffSlider->value(50);

	Fl_Box* temporalCutoffSliderLabel = new Fl_Box(10, 250 + temporalYOffset, 100, 20, "Temporal Cutoff: 50");

	Fl_Button* genTemporalButton = new Fl_Button(130, 250 + temporalYOffset, 180, 25, "Generate Temporal");

	TEMPORAL_VIEW_UPDATE temp = { temporalCutoffSliderLabel, imageBox};

	temporalCutoffSlider->callback(temporalSliderCallBack, &temp);
	genTemporalButton->callback(temporalGenerateCallBack, &temp);

	// Optical Flow final image
	Fl_PNG_Image velImage("1WithVelocites.png");
	Fl_Box* velImageBox = new Fl_Box(300, 0 + temporalYOffset, 270, 180);
	Fl_Image* vel_scaled_image = velImage.copy(270, 180);
	velImageBox->image(vel_scaled_image);

	Fl_Button* genOptics = new Fl_Button(320, 250 + temporalYOffset, 180, 25, "Generate Optical Flow");
	genOptics->callback(opticalGenerateCallBack, velImageBox);


	Fl::lock();

	window->end();
	window->show();

	return Fl::run();
}


long double scaleToTensPlace(long double num) {
	if (num == 0.0L) {
		return 0.0L;
	}
	long double sign = (num < 0.0L) ? -1.0L : 1.0L;
	num = std::abs(num);
	while (num < 10.0L) {
		num *= 10.0L;
	}

	return sign * num;
}

void GenerateTemporal(std::string f1, std::string f2) {
	Frame frame1(f1);
	Frame frame2(f2);

	printf("FRAME 1(%d, %d) | FRAME 2(%d, %d)\n", frame1.getWidth(), frame1.getHeight(), frame2.getWidth(), frame2.getHeight());

	int width = frame1.getWidth();
	int height = frame2.getHeight();

	Frame temporal("temporal.png", width, height);
	Frame f1Gray("f1gray.png", width, height);

	assert(frame1.getHeight() == frame2.getHeight());
	assert(frame1.getWidth() == frame2.getWidth());

	// Preform Lucas-Kande Optical Flow

	printf("Begin Temporal Gradient calculation\n");
	// Get temporal gradient (just the brightness lmao)
	for (int y = 0; y < height; y++) {
		PixelRow f1Row = *(frame1.getRow(y));
		PixelRow f2Row = *(frame2.getRow(y));
		PixelRow temRow = *(temporal.getRow(y));
		PixelRow grayRow = *(f1Gray.getRow(y));
		for (int x = 0; x < f1Row.getSize(); x++) {
			// Calculate first brigness
			Pixel p1 = f1Row[x];
			double oneIntensity = (p1.r * rIntensityConstant) + (p1.g * gIntensityConstant) + (p1.b * bIntensityConstant);

			Pixel g = { oneIntensity, oneIntensity, oneIntensity };
			grayRow.setPixel(g, x);

			// Calculate second brigness
			Pixel p2 = f2Row[x];
			double twoIntensity = (p2.r * rIntensityConstant) + (p2.g * gIntensityConstant) + (p2.b * bIntensityConstant);

			// Calculate difference
			double dI = oneIntensity - twoIntensity;

			Pixel tmp = { dI, dI, dI };
			if (dI < 0) tmp = { 0, dI, 255 };
			if (std::abs(dI) < temporalCutoff) {
				tmp = { 0, 0, 0 };
			}
			temRow.setPixel(tmp, x);

		}
		//printf("%f percent complete\r", (((double)y / (double)(height - 1)) * 100.0));
		temporal.setRow(temRow, y);
		f1Gray.setRow(grayRow, y);
	}

	temporal.Write();
	f1Gray.Write();
}

void OpticalFlow(std::string f1, std::string f2) {
	Frame frame1(f1);
	Frame frame2(f2);

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
	for (int y = 0; y < height; y++) {
		PixelRow f1Row = *(frame1.getRow(y));
		PixelRow f2Row = *(frame2.getRow(y));
		PixelRow temRow = *(temporal.getRow(y));
		PixelRow grayRow = *(f1Gray.getRow(y));
		for (int x = 0; x < f1Row.getSize(); x++) {
			// Calculate first brigness
			Pixel p1 = f1Row[x];
			double oneIntensity = (p1.r * rIntensityConstant) + (p1.g * gIntensityConstant) + (p1.b * bIntensityConstant);

			Pixel g = { oneIntensity, oneIntensity, oneIntensity };
			grayRow.setPixel(g, x);

			// Calculate second brigness
			Pixel p2 = f2Row[x];
			double twoIntensity = (p2.r * rIntensityConstant) + (p2.g * gIntensityConstant) + (p2.b * bIntensityConstant);

			// Calculate difference
			double dI = oneIntensity - twoIntensity;

			Pixel tmp = { dI, dI, dI };
			if (dI < 0) tmp = { 0, dI, 255 };
			if (std::abs(dI) < temporalCutoff) {
				tmp = { 0, 0, 0 };
			}
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

	frame1.setPath("1WithVelocites.png");

	//frame1.drawLine(150, 200, 50, 25);

	frame1.Write();

	for (int y = 1; y < height - 2; y++) {
		PixelRow grayRow = *(hack.getRow(y));
		PixelRow grayRowPrev = *(hack.getRow(y - 1));
		PixelRow grayRowAfter = *(hack.getRow(y + 1));
		PixelRow sobelRow = *(sobel.getRow(y - 1));
		PixelRow temporRow = *(temporal.getRow(y));

		PixelRow frame1VelRow = *(frame1.getRow(y));

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


			for (int i = 0; i < 9; i++) {
				atB[0][0] += Amatrix[i][0] * temporRow[x].g;
				atB[1][0] += Amatrix[i][1] * temporRow[x].g;
			}

			long double aInv[2][2]{};
			// 

			long double invConst = 1 / ((atA[0][0] * atA[1][1]) - (atA[0][1] * atA[1][0]));

			long double velocityX = 0, velocityY = 0;

			double regularization = 1e-3;
			long double determinant = (atA[0][0] * atA[1][1]) - (atA[0][1] * atA[1][0]) + regularization;
			if (std::abs(determinant) < 1e-6 || std::isinf(invConst)) {
				velocityX = velocityY = 0;
				printf("%f percent complete\r", (((double)y / (double)(height - 2)) * 100.0));
			}
			else {
				long double invConst = 1.0 / determinant;
				aInv[0][0] = invConst * atA[1][1];
				aInv[0][1] = -invConst * atA[0][1];
				aInv[1][0] = -invConst * atA[1][0];
				aInv[1][1] = invConst * atA[0][0];

				velocityX = (aInv[0][0] * atB[0][0]) + (aInv[0][1] * atB[1][0]);
				velocityY = (aInv[1][0] * atB[0][0]) + (aInv[1][1] * atB[1][0]);

				/*if (std::abs(velocityX) < 1e-3) {
					velocityX = (velocityX < 0 ? -1e-3 : 1e-3);
				}
				if (std::abs(velocityY) < 1e-3) {
					velocityY = (velocityY < 0 ? -1e-3 : 1e-3);
				}*/

				velocityX = scaleToTensPlace(velocityX) / 5;
				velocityY = scaleToTensPlace(velocityY) / 5;

				Pixel tmp = { frame1VelRow[x].r, (atB[1][0] + frame1VelRow[x].g) / 2, (atB[0][0] + frame1VelRow[x].g) / 2 };
				frame1.getRow(y)->setPixel(tmp, x);

				if (x % 10 == 0 && y % 10 == 0 && (velocityX != 0 || velocityY != 0))
					frame1.drawLine(x, y, velocityX, velocityY);

				printf("%f percent complete | VelX %f VelY %f\r", (((double)y / (double)(height - 2)) * 100.0), velocityX, velocityY);
			}


			//printf("%f %f\n%f %f\r", atA[0][0], atA[0][1], atA[1][0], atA[1][1]);



			Pixel sobelTmp = { accX, accY, 0 };
			sobelRow.setPixel(sobelTmp, x - 1);

		}

		sobel.setRow(sobelRow, y - 1);
	}

	sobel.Write();

	// Draw velocity arrows


	frame1.Write();
}