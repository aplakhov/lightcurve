#include <stdlib.h>
#include "CImg/CImg.h"
typedef unsigned char ui8;
typedef cimg_library::CImg<ui8> TImage;

const float u = 0.65f;
const int R = 220;

static float Luminosity(int x, int y) {
    float r2 = (x - R)*(x - R) + (y - R)*(y - R);
    float r = sqrt(r2);
    if (r <= R) {
        // I used the "real" photosphere luminosity formula given e.g. here: http://crydee.sai.msu.ru/ak4/Chapt_9_121.htm
        // Sorry for the link in Russian, I couldn't find an English equivalent online

        // We could just write "return 255" for simplicity here, but we want a planetary transit to resemble an image from this page:
        // https://en.wikipedia.org/wiki/Methods_of_detecting_exoplanets#Transit_photometry
        // and with "return 255" we'd get a flat bottom

        // Also, the star looks much more natural and visually pleasing this way
        return 255 * (1 - u + u * sqrt(1 - (r / R)*(r / R)));
    } else {
        return 0;
    }
}

int main()
{
    const int borderX = 250, borderY = 17;
    const int graphHeight = 200;

    TImage background(2*R + 2*borderX, 2*R + 2*borderY + graphHeight, 1, 3);
    background.fill(0);

    TImage object("object.bmp");

    // just a decoration -- people expect stars in space 
    for (int star = 0; star < 600; ++star) {
        int brightness = rand() % 255;
        int x = rand() % background.width();
        int y = rand() % background.height();
        background(x, y, 0, 0) = brightness;
        background(x, y, 0, 1) = brightness;
        background(x, y, 0, 2) = brightness / 2;
    }

    // the star itself
    double totalLuminosity = 0;
    for (int x = 0; x < 2*R; ++x) {
        for (int y = 0; y < 2*R; ++y) {
            float luminosity = Luminosity(x, y);
            if (luminosity > 0) {
                background(x + borderX, y + borderY, 0, 0) = (int)luminosity;
                background(x + borderX, y + borderY, 0, 1) = (int)luminosity;
                background(x + borderX, y + borderY, 0, 2) = (int)luminosity / 2;
                totalLuminosity += luminosity;
            }
        }
    }

    TImage img(background.width(), background.height(), 1, 3);
    cimg_library::CImgDisplay display(background, "BTW, Gagarin was a superhero");

    const float graphTimeScale = (object.width() + img.width()) / (float)background.width();
    TImage graph(background.width(), graphHeight, 1, 3);
    graph.fill(0);

    int tick = 0;
    while (!display.is_closed()) {
        img.draw_image(background);
        double totalFrameLuminosity = 0;
        for (int x = 0; x < img.width(); ++x) {
            for (int y = 0; y < 2 * R; ++y) {
                int objX = x - tick + object.width();
                if (objX > 0 && objX < object.width() && object(objX, y, 0, 0)) {
                    int gray = (objX * 59 + y * 239) % 16;
                    img(x, y + borderY, 0, 0) = gray;
                    img(x, y + borderY, 0, 1) = gray;
                    img(x, y + borderY, 0, 2) = gray / 2;
                } else {
                    totalFrameLuminosity += Luminosity(x - borderX, y);
                }
            }
        }
        int graphX = tick / graphTimeScale;
        int graphY = 3.5f * graphHeight - ((totalFrameLuminosity / totalLuminosity) * (3.5f * graphHeight - 10));
        const ui8 red[] = { 255, 0, 0 };
        const ui8 white[] = { 128, 128, 128 };
        graph.draw_circle(graphX, graphY, 1, red, 1);
        img.draw_image(0, 2 * R + 2 * borderY, graph, 1);
        graph.draw_circle(graphX, graphY, 1, white, 1);
        img.display(display);
        ++tick;
        if (tick > object.width() + img.width()) {
            tick = 0;
            graph.fill(0);
        }
    }
    return 0;
}
