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


#ifndef MULTIVIEW_H_INCLUDED
#define MULTIVIEW_H_INCLUDED


#define MAX_CHAR_SEQ    6



#include <opencv2/core/utility.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"


#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

// Examples of function poiter:     https://stackoverflow.com/questions/2225330/member-function-pointers-with-default-arguments
// How to create a type of function pointer;
typedef int  TypeFuncNoPar();
typedef void TypeFuncWhPar(viod&);


extern int is_there_gui;
extern int run_from_cl;


#endif // MULTIVIEW_H_INCLUDED
