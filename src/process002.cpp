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
///              https://docs.opencv.org/3.4/db/d28/tutorial_cascade_classifier.html




#include "process002.h"

void process002(viod &vd){      // It detects faces from images

    ///vd.buffers[vd.bon].start,vd.buffers[vd.bon].length, vd.h, vd.w

    CascadeClassifier* pface_cascade;
    CascadeClassifier* peyes_cascade;

    if(!vd.procinit){       /// Must be initialized only once

        /// It deletes images and matrices that were previously initialized
        erase_process_initialization(vd);

        /// Crate new images                            /// rgb image  (0)
        vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC1));     /// gray image (1)
        //vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC3));     /// output image (2)


        /// Create new matrices
        //vd.c_mat.push_back(Mat(vd.h,vd.w));     /// matrix for calculation (0)


        /// Create other types and register theirs
        pface_cascade = new(CascadeClassifier);
        vd.d_vet.push_back(CREATE_DANY(pface_cascade,CascadeClassifier));   /// pointer to type CascadeClassifier(0)
        peyes_cascade = new(CascadeClassifier);
        vd.d_vet.push_back(CREATE_DANY(peyes_cascade,CascadeClassifier));   /// pointer to type CascadeClassifier(1)




        /// Make just one time

        samples::addSamplesDataSearchPath("/home/josemar/projects/linkopencv/data/haarcascades");
        //String face_cascade_name = samples::findFile("haarcascade_frontalface_alt.xml",1,1);
        //String eyes_cascade_name = samples::findFile("haarcascade_eye_tree_eyeglasses.xml",1,1);

        String face_cascade_name = samples::findFile("haarcascade_frontalface_default.xml",1,1);
        String eyes_cascade_name = samples::findFile("haarcascade_eye.xml",1,1);

        //-- 1. Load the cascades
        if( !(*pface_cascade).load( face_cascade_name ) ){

            cout << "--(!)Error loading face cascade\n";

        }

        if( !(*peyes_cascade).load( eyes_cascade_name ) ){
            cout << "--(!)Error loading eyes cascade\n";

        }


        /// Don't repeat it
        vd.procinit = 1;
    }



    cvtColor(vd.v_mat.at(0), vd.v_mat.at(1), COLOR_BGR2GRAY);
    equalizeHist(vd.v_mat.at(1), vd.v_mat.at(1));

    //-- Detect faces
    std::vector<Rect> faces;
    pface_cascade =  (CascadeClassifier*)vd.d_vet.at(0).pobj;
    (*pface_cascade).detectMultiScale( vd.v_mat.at(1), faces );

    //vd.v_mat.at(2) = vd.v_mat.at(0).clone();

    for ( size_t i = 0; i < faces.size(); i++ ){

        Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
        Mat faceROI = vd.v_mat.at(1)( faces[i] );
        //ellipse( vd.v_mat.at(0), center, Size( faces[i].width/2, faces[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 255 ), 4 );
        rectangle(vd.v_mat.at(0), faces[i], Scalar( 255, 0, 0 ), 2, 1 );

        //-- In each face, detect eyes
        std::vector<Rect> eyes;
        peyes_cascade =  (CascadeClassifier*)vd.d_vet.at(1).pobj;
        (*peyes_cascade).detectMultiScale( faceROI, eyes );

        for ( size_t j = 0; j < eyes.size(); j++ ){

            Point eye_center( faces[i].x + eyes[j].x + eyes[j].width/2, faces[i].y + eyes[j].y + eyes[j].height/2 );
            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( vd.v_mat.at(0), eye_center, radius, Scalar( 0, 255, 0 ), 4 );

        }

    }

}
