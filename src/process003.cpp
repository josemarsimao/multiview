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




#include "process003.h"

void process003(viod &vd){      // It detects smiles

    ///vd.buffers[vd.bon].start,vd.buffers[vd.bon].length, vd.h, vd.w


    CascadeClassifier* psmilecas;

    if(!vd.procinit){       /// Must be initialized only once

        /// It deletes images and matrices that were previously initialized
        erase_process_initialization(vd);

        /// Crate new images                            /// rgb image  (0)
        vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC1));     /// gray image (1)
        //vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC3));     /// output image (2)

        /// Create other types and register theirs
        // Create CascadeClassifier
        psmilecas = new(CascadeClassifier);
        vd.d_vet.push_back(CREATE_DANY(psmilecas,CascadeClassifier));



        /// Make just one time
        //samples::addSamplesDataSearchPath("~/projects/linkopencv/data/haarcascades");
        //String smile_cascade_name = samples::findFile("haarcascade_smile.xml",1,1);


        //-- 1. Load the cascades
        //if( !smile_cascade.load( smile_cascade_name ) ){

        if( !(*psmilecas).load( "/usr/local/share/opencv4/haarcascades/haarcascade_smile.xml" ) ){


            cout << "--(!)Error loading face cascade\n";

        }





        /// Don't repeat it
        vd.procinit = 1;
    }

    cvtColor(vd.v_mat.at(0), vd.v_mat.at(1), COLOR_BGR2GRAY);
    equalizeHist(vd.v_mat.at(1), vd.v_mat.at(1));

    //-- Detect faces
    std::vector<Rect> smiles;
    psmilecas =  (CascadeClassifier*)vd.d_vet.at(0).pobj;
    (*psmilecas).detectMultiScale( vd.v_mat.at(1), smiles );

    //vd.v_mat.at(2) = vd.v_mat.at(0).clone();

    for ( size_t i = 0; i < smiles.size(); i++ ){

        Point center( smiles[i].x + smiles[i].width/2, smiles[i].y + smiles[i].height/2 );
        ellipse( vd.v_mat.at(0), center, Size( smiles[i].width/2, smiles[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 255 ), 4 );

    }

}
