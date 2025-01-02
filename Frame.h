/*
    Holds a single image in memory
*/
#ifndef FRAME_H
#define FRAME_H
#include <string>
#include <vector>

#include <png++/png.hpp>

typedef struct {
    double r, g, b;
} Pixel;

class PixelRow {
private:
    std::vector<Pixel> pixels;
public:
    void addPixel(Pixel p);

    Pixel operator[](int i);

    void setPixel(Pixel p, int x);

    PixelRow operator=(PixelRow* other);

    int getSize();
};

class Frame {
private:
    int width, height;

    std::string path;

    png::image<png::rgb_pixel> image;

    std::vector<PixelRow> pixels;
public:
    /*
        Load image
    */
    Frame(std::string);

    /*
        Create new Image
    */
    Frame(std::string, int, int);

    PixelRow* getRow(int y);

    void setRow(PixelRow row, int y);

    void Write();

    int getWidth();

    int getHeight();

    void drawLine(int x, int y, double xV, double yV);

    std::string getPath();

    void setPath(std::string path);

    PixelRow* operator[](int i);


};
#endif