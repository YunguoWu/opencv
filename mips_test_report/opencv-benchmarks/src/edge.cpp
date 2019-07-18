#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include <stdio.h>

#define MAX_TRESH 50
#define MAX_THRESHSCHARR 200
using namespace cv;
using namespace std;

int edgeThresh = 1;
int edgeThreshScharr=1;

Mat image, gray, blurImage, edge1, edge2, cedge;

// define a trackbar callback
static void onTrackbar(int, void*)
{
    // uncoment for debug
    //printf("\n Testing with: \n");
    //printf("edgeThresh = %d\n",edgeThresh);
    //printf("edgeThreshScharr = %d\n",edgeThreshScharr);

    blur(gray, blurImage, Size(3,3));

    // Run the edge detector on grayscale
    Canny(blurImage, edge1, edgeThresh, edgeThresh*3, 3);
    cedge = Scalar::all(0);

    image.copyTo(cedge, edge1);
    imwrite( "edge_1_image.jpg", cedge);

    /// Canny detector with scharr
    Mat dx,dy;
    Scharr(blurImage,dx,CV_16S,1,0);
    Scharr(blurImage,dy,CV_16S,0,1);
    Canny( dx,dy, edge2, edgeThreshScharr, edgeThreshScharr*3 );
    /// Using Canny's output as a mask, we display our result
    cedge = Scalar::all(0);
    image.copyTo(cedge, edge2);
    imwrite( "edge_2_image.jpg", cedge);
}

static void help()
{
   printf("\n ==== OPENCV BENCHMARK ==== \n");
   printf("\nThis sample demonstrates Canny edge detection\n"
           "Call:\n"
           "    /.edge [image_name -- Default is ../data/fruits.jpg]\n\n");
}

const char* keys =
{
    "{help h||}{@image |lena.jpg|input image name}"
};

int main( int argc, const char** argv )
{
    help();
    CommandLineParser parser(argc, argv, keys);
    string filename = parser.get<string>(0);

    image = imread(filename, IMREAD_COLOR);
    if(image.empty())
    {
        printf("Cannot read image file: %s\n", filename.c_str());
        help();
        return -1;
    }
    cedge.create(image.size(), image.type());
    cvtColor(image, gray, COLOR_BGR2GRAY);


    // create a toolbar
    for (int value_Thresh=1; value_Thresh < MAX_TRESH; value_Thresh++){
        for(int value_ThreshScharr=1 ; value_ThreshScharr < MAX_THRESHSCHARR; value_ThreshScharr++){
            edgeThresh = value_Thresh;
            edgeThreshScharr = value_ThreshScharr;
            onTrackbar(0, 0);
        }
    }
    return 0;
}
