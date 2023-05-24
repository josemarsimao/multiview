/*
 *  V4L2 video capture My tests
 *
 *	Nome: Josemar Simão
 */

/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 */

/// Reference:
///              https://docs.opencv.org/4.x/d7/d00/tutorial_meanshift.html




#include "process005.h"


float range_[] = {0, 180};
const float* range[] = {range_};
Mat roi_hist;
int histSize[] = {180};
int channels[] = {0};

// Setup the termination criteria, either 10 iteration or move by at least 1 pt
TermCriteria term_crit(TermCriteria::EPS | TermCriteria::COUNT, 10, 1);

Rect r;


void process005(viod &vd){      // It tracks a rectagle choosed from first frame using CamShift algorithm



    if(!vd.procinit){       /// Must be initialized only once

        /// It deletes images and matrices that were previously initialized
        erase_process_initialization(vd);

        /// Crate new images                            /// rgb image  (0)
        vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC3));     /// hsv image  (1)
        vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC3));     /// dst image  (2)



        /// Create new matrices
        //vd.c_mat.push_back(Mat(vd.h,vd.w));     /// matrix for calculation (0)


        /// Create other types and register them
        //Rect2d* pbox = new(Rect2d);
        //vd.d_vet.push_back(CREATE_DANY(pbox,Rect2d));                       /// pointer to type Rect2b(0)





        /// Make some initializations
        // Select ROI
        //*(Rect2d*)vd.d_vet.at(0).pobj = selectROI("Select", vd.v_mat.at(0));
        r = selectROI("Select", vd.v_mat.at(0));
        destroyWindow("Select");

        //Mat roi = vd.v_mat.at(0)(*(Rect2d*)vd.d_vet.at(0).pobj);
        Mat roi = vd.v_mat.at(0)(r);
        Mat hsv_roi, mask;
        cvtColor(roi, hsv_roi, COLOR_BGR2HSV);
        inRange(hsv_roi, Scalar(0, 60, 32), Scalar(180, 255, 255), mask);
        calcHist(&hsv_roi, 1, channels, mask, roi_hist, 1, histSize, range);
        normalize(roi_hist, roi_hist, 0, 255, NORM_MINMAX);



        /// Don't repeat it
        vd.procinit = 1;
    }



    cvtColor(vd.v_mat.at(0), vd.v_mat.at(1), COLOR_BGR2HSV);
    calcBackProject(&vd.v_mat.at(1), 1, channels, roi_hist, vd.v_mat.at(2), range);

    // apply meanshift to get the new location


    //r.height = ((Rect2d*)vd.d_vet.at(0).pobj)->height;
    //r.width = ((Rect2d*)vd.d_vet.at(0).pobj)->width;

    // apply camshift to get the new location
    RotatedRect rot_rect = CamShift(vd.v_mat.at(2), r, term_crit);

    // Draw it on image
    Point2f points[4];
    rot_rect.points(points);
    for (int i = 0; i < 4; i++)
        line(vd.v_mat.at(0), points[i], points[(i+1)%4], 255, 2);


    //meanShift(vd.v_mat.at(2), r, term_crit);

    // Draw it on image
    //rectangle(vd.v_mat.at(0), *(Rect2d*)vd.d_vet.at(0).pobj, 255, 2);
    //rectangle(vd.v_mat.at(0), r, 255, 2);








}
