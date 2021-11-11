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

#include "process001.h"

void process001(viod &vd){      // It detects lines from images

    ///vd.buffers[vd.bon].start,vd.buffers[vd.bon].length, vd.h, vd.w

    if(!vd.procinit){       /// Must be initialized only once

        /// It deletes images and matrices that were previously initialized
        erase_process_initialization(vd);

        /// Crate new images                            /// rgb image  (0)
        vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC1));     /// gray image (1)
        vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC1));     /// edge image (2)
//        vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC3));     /// line image (3)


        vd.c_mat.push_back(Mat(vd.h,vd.w,CV_8UC1));     /// calc matrix (0)


        /// Don't repeat it
        vd.procinit = 1;
    }



    cvtColor(vd.v_mat.at(0), vd.v_mat.at(1), COLOR_BGR2GRAY);
    Canny(vd.v_mat.at(1), vd.v_mat.at(2), 50, 200);

    vector<Vec4i> linesP; // will hold the results of the detection
    HoughLinesP(vd.v_mat.at(2), linesP, 1, CV_PI/180, 50, 50, 10 ); // runs the actual detection


    //memset(vd.v_mat.at(3).data,0,vd.h*vd.w*3);
    // Draw the lines
    for( size_t i = 0; i < linesP.size(); i++ )
    {
        Vec4i l = linesP[i];
        line( vd.v_mat.at(0), Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 1, LINE_AA);
    }

}
