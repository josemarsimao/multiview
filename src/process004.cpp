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

// Reference:
//              https://www.learnopencv.com/object-tracking-using-opencv-cpp-python/
///             https://docs.opencv.org/4.3.0/d2/d0a/tutorial_introduction_to_tracker.html


#include "process004.h"

/// TRACKER TYPES  ---------------------------------
//   1 - cv::TrackerBoosting
//   2 - cv::TrackerMIL
//   3 - cv::TrackerKCF
//   4 - cv::TrackerTLD
//   5 - cv::TrackerMedianFlow
//   6 - cv::TrackerGOTURN
//   7 - cv::TrackerMOSSE
//   8 - cv::TrackerCSRT
/// ---------------------------------------------------

#define TRACKER_TYPE   2

static Ptr<Tracker> CreateTracker(){
#if(TRACKER_TYPE == 1)
    return cv::TrackerBoosting::create();
#elif(TRACKER_TYPE == 2)
    return cv::TrackerMIL::create();                /// good
#elif(TRACKER_TYPE == 3)
    return cv::TrackerKCF::create();
#elif(TRACKER_TYPE == 4)
    return cv::TrackerTLD::create();                /// detection error
#elif(TRACKER_TYPE == 5)
    return cv::TrackerMedianFlow::create();         /// reference error
#elif(TRACKER_TYPE == 6)
    return cv::TrackerGOTURN::create();             /// doesnt work (need more information)
#elif(TRACKER_TYPE == 7)
    return cv::TrackerMOSSE::create();              /// reference error
#elif(TRACKER_TYPE == 8)
    return cv::TrackerCSRT::create();               /// good
#endif
}


void process004(viod &vd){      // It detects smiles

    ///vd.buffers[vd.bon].start,vd.buffers[vd.bon].length, vd.h, vd.w

    Ptr<Tracker>* ptracker;

    if(!vd.procinit){       /// Must be initialized only once

        /// It deletes images and matrices that were previously initialized
        erase_process_initialization(vd);

        /// Create new images                            /// rgb image  (0)
        //vd.v_mat.push_back(Mat(vd.h,vd.w,CV_8UC3));     /// output image (1)


        /// Create new matrices
        //vd.c_mat.push_back(Mat(vd.h,vd.w));     /// matrix for calculation (0)


        /// Create other types and register theirs
        // Create Rect
        Rect2d* pbox = new(Rect2d);
        vd.d_vet.push_back(CREATE_DANY(pbox,Rect2d));                       /// pointer to type Rect2b(0)

        /*      ENTENDENDO O LAMBDA EM C++
        É um encapsulamento de umas poucas linhas que código que pode ser passado como parâmetro
        no exemplo: " [](const void* x) { static_cast<const Rect2d*>(x)->~Rect2d(); } " todo este bloco
        é o ponteiro de uma função declarada com lambda "[]". Este ponteiro é conhecido e utilizado para
        preencher o segundo membro da estrutura. Neste exemplo uma função do tipo lambda é criada para
        acessar a função destrutor dos objetos criados, uma vez que não é permitido obter o endereço
        da função destrutor. O tipo do objeto é passado para que as funções lambdas sejam criadas para
        cada tipo. O termo "(const void* x)" refere-se ao parâmentro que será passado para a função
        lambda. Este será convertido (cast) para o tipo que queremos destruir.

        Outros exemplos:

        typedef void(*pF)(const void*);     Declaração de um ponteiro para função que retorna void e recebe
                                            como parãmetro um "const void*". Por isso que na declaração
                                            do lambda abaixo aparece um " [](const void*) ".

        pF dsc = [](const void* x) { static_cast<const Rect2d*>(x)->~Rect2d(); };

                                            Aqui, o  " x " que é passado como parâmetro é convertido (cast)
                                            para "const Rect2d*" para poder acessar o seu destrutor ~Rect2d()

        typedef void(*pF)();                Declaração de um ponteiro para função que retorna void e não
                                            recebe nenhum parãmetro.

        pF dsc = []() { Rect2d* x; x->~Rect2d(); };

        */

        // Create Ptr<Tracker> and Initialize it
        ptracker = new(Ptr<Tracker>);
        *ptracker = CreateTracker();
        vd.d_vet.push_back(CREATE_DANY(ptracker,Ptr<Tracker>));              /// pointer to type Tracker(1)


        /// Make some initializations

        // Select ROI
        //rbox = selectROI(vd.v_mat.at(0));
        *(Rect2d*)vd.d_vet.at(0).pobj = selectROI("Select", vd.v_mat.at(0));
        destroyWindow("Select");

        // Initialize the tracker
        //tracker->init(vd.v_mat.at(0), rbox);
        (*(Ptr<Tracker>*)vd.d_vet.at(1).pobj)->init(vd.v_mat.at(0), *(Rect2d*)vd.d_vet.at(0).pobj);







        /// Don't repeat it
        vd.procinit = 1;
    }



    //int ok = tracker->update(vd.v_mat.at(0), rbox);
    ptracker = (Ptr<Tracker>*)vd.d_vet.at(1).pobj;
    int ok = (*ptracker)->update(vd.v_mat.at(0), *(Rect2d*)(vd.d_vet.at(0).pobj));

    if(ok){
        // Tracking success : Draw the tracked object
        //rectangle(vd.v_mat.at(0), rbox, Scalar( 255, 0, 0 ), 2, 1 );
        rectangle(vd.v_mat.at(0), *(Rect2d*)(vd.d_vet.at(0).pobj), Scalar( 255, 0, 0 ), 2, 1 );

    }else{
        // Tracking failure detected.
        putText(vd.v_mat.at(0), "Tracking failure detected", Point(100,80), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0,0,255),2);
    }

}
