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

#include "process000.h"

void process000(viod &vd){      // It converts color images to gray images

    ///vd.buffers[vd.bon].start,vd.buffers[vd.bon].length, vd.h, vd.w

    if(!vd.procinit){       /// Must be initialized only once

        /// It deletes images and matrices that were previously initialized
        erase_process_initialization(vd);

        /// Crate new images                            /// rgb image  (0)
        vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC1));     /// gray image (1)
        vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC1));     /// edge image (2)


        /// Don't repeat it
        vd.procinit = 1;
    }




    cvtColor(vd.v_mat.at(0), vd.v_mat.at(1), COLOR_BGR2GRAY);
    Canny(vd.v_mat.at(1), vd.v_mat.at(2), 100, 200);

    cvtColor(vd.v_mat.at(2), vd.v_mat.at(0), COLOR_GRAY2BGR);

}
