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

///        |  (O)pen  | (G)et     | (P)rocess |           |
///        |          | (S)et     | (V)iew    |           |
///        |          | (C)apture | (E)xit    |           |
///        |          |           |           |           |
///        |          |           |           |           |
///        |          |           |           |           |
///        |          |           |           |           |
/// Status:| CLOSED   | OPEN      | CAPTURING | PROCESSING|
///        |          |           |           |           |
///        | list of  |           |           |           |
///        | cameras  |           |           |           |
///        | that can |           |           |           |
///        | be open  |           |           |           |





#include <mutex>


#include "camview.h"
#include "v4l2misc.h"
#include "kbhit.h"
#include <pthread.h>
#include "multiview.h"

#include "process000.h"
#include "process001.h"
#include "process002.h"
#include "process003.h"
#include "process004.h"

int is_there_gui;
int run_from_cl;

mutex m;

char title[] = "IoT - Image on Terminal";
char date[] = "dd/mm/aaaa";
char hour[] = "hh:mm:ss";





///void process_image(const void *p, int isize) {
void process_image(const void *p, int isize){}







TypeFuncWhPar *vet_of_funcs[] = {&process000, &process001, &process002, &process003, &process004, 0};




/// source: OPenCv Code
/// https://github.com/opencv/opencv/blob/master/modules/videoio/src/cap_v4l.cpp
/// ****************************************************************************
/// libv4lconvert has a more detailed version


/*
 * Turn a YUV4:2:0 block into an RGB block
 *
 * Video4Linux seems to use the blue, green, red channel
 * order convention-- rgb[0] is blue, rgb[1] is green, rgb[2] is red.
 *
 * Color space conversion coefficients taken from the excellent
 * http://www.inforamp.net/~poynton/ColorFAQ.html
 * In his terminology, this is a CCIR 601.1 YCbCr -> RGB.
 * Y values are given for all 4 pixels, but the U (Pb)
 * and V (Pr) are assumed constant over the 2x2 block.
 *
 * To avoid floating point arithmetic, the color conversion
 * coefficients are scaled into 16.16 fixed-point integers.
 * They were determined as follows:
 *
 *  double brightness = 1.0;  (0->black; 1->full scale)
 *  double saturation = 1.0;  (0->greyscale; 1->full color)
 *  double fixScale = brightness * 256 * 256;
 *  int rvScale = (int)(1.402 * saturation * fixScale);
 *  int guScale = (int)(-0.344136 * saturation * fixScale);
 *  int gvScale = (int)(-0.714136 * saturation * fixScale);
 *  int buScale = (int)(1.772 * saturation * fixScale);
 *  int yScale = (int)(fixScale);
 */

/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

static inline void move_411_block(int yTL, int yTR, int yBL, int yBR, int u, int v, int /*rowPixels*/, unsigned char * rgb){

    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
    //  if (force_rgb) {
    //      r = buScale * u;
    //      b = rvScale * v;
    //  } else {
    r = rvScale * v;
    b = buScale * u;
    //  }

    yTL *= yScale; yTR *= yScale;
    yBL *= yScale; yBR *= yScale;

    /* Write out top two first pixels */
    rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
    rgb[2] = LIMIT(r+yTL);

    rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
    rgb[5] = LIMIT(r+yTR);

    /* Write out top two last pixels */
    rgb += 6;
    rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
    rgb[2] = LIMIT(r+yBL);

    rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
    rgb[5] = LIMIT(r+yBR);
}

static inline void move_m420_block(int yTL, int yTR, int yBL, int yBR, int u, int v, int width, unsigned char * rgb){

    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
    //  if (force_rgb) {
    //      r = buScale * u;
    //      b = rvScale * v;
    //  } else {
    r = rvScale * v;
    b = buScale * u;
    //  }

    yTL *= yScale; yTR *= yScale;
    yBL *= yScale; yBR *= yScale;

    /* Write out top two first pixels */
    rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
    rgb[2] = LIMIT(r+yTL);

    rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
    rgb[5] = LIMIT(r+yTR);

    /* Write out top two last pixels */
    rgb += (3*width);
    rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
    rgb[2] = LIMIT(r+yBL);

    rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
    rgb[5] = LIMIT(r+yBR);
}

// Consider a YUV411P image of 8x2 pixels.
//
// A plane of Y values as before.
//
// A plane of U values    1       2
//                        3       4
//
// A plane of V values    1       2
//                        3       4
//
// The U1/V1 samples correspond to the ABCD pixels.
//     U2/V2 samples correspond to the EFGH pixels.
//
/* Converts from planar YUV411P to RGB24. */
/* [FD] untested... */
static void yuv411p_to_rgb24(int width, int height, unsigned char *pIn0, unsigned char *pOut0){

    const int numpix = width * height;
    const int bytes = 24 >> 3;
    int i, j, y00, y01, y10, y11, u, v;
    unsigned char *pY = pIn0;
    unsigned char *pU = pY + numpix;
    unsigned char *pV = pU + numpix / 4;
    unsigned char *pOut = pOut0;

    for (j = 0; j <= height; j++) {
        for (i = 0; i <= width - 4; i += 4) {
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + 2);
            y11 = *(pY + 3);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_411_block(y00, y01, y10, y11, u, v, width, pOut);

            pY += 4;
            pOut += 4 * bytes;

        }
    }
}

/// V4L2_PIX_FMT_M420 to V4L2_PIX_FMT_BGR24
static void m420_to_rgb24(int width, int height, unsigned char *pIn0, unsigned char *pOut0){

    const int dbW = width * 2;
    const int bytes = 24 >> 3;
    int i, j, y00, y01, y10, y11, u, v;

    unsigned char *pOut = pOut0;

    unsigned char *pY = pIn0;




    for (j = 0; j <= height - 2; j += 2) {
        for (i = 0; i <= width - 2; i += 2) {
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + width);
            y11 = *(pY + width + 1);
            u = *(pY+dbW) - 128;
            v = *(pY+dbW+1) - 128;

            move_m420_block(y00, y01, y10, y11, u, v, width, pOut);

            pY += 2;
            pOut += 2 * bytes;

        }
        pY += dbW;
        pOut += (3 * width);
    }

    pOut = pOut;
}

/*
 * BAYER2RGB24 ROUTINE TAKEN FROM:
 *
 * Sonix SN9C10x based webcam basic I/F routines
 * Takafumi Mizuno <taka-qce@ls-a.jp>
 *
 */
static void bayer2rgb24(long int WIDTH, long int HEIGHT, unsigned char *src, unsigned char *dst){

    long int i;
    unsigned char *rawpt, *scanpt;
    long int size;

    rawpt = src;
    scanpt = dst;
    size = WIDTH*HEIGHT;

    for ( i = 0; i < size; i++ ) {
        if ( (i/WIDTH) % 2 == 0 ) {
            if ( (i % 2) == 0 ) {
                /* B */
                if ( (i > WIDTH) && ((i % WIDTH) > 0) ) {
                    *scanpt++ = (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+
                            *(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;  /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1)+
                            *(rawpt+WIDTH)+*(rawpt-WIDTH))/4;      /* G */
                    *scanpt++ = *rawpt;                                     /* B */
                } else {
                    /* first line or left column */
                    *scanpt++ = *(rawpt+WIDTH+1);           /* R */
                    *scanpt++ = (*(rawpt+1)+*(rawpt+WIDTH))/2;      /* G */
                    *scanpt++ = *rawpt;                             /* B */
                }
            } else {
                /* (B)G */
                if ( (i > WIDTH) && ((i % WIDTH) < (WIDTH-1)) ) {
                    *scanpt++ = (*(rawpt+WIDTH)+*(rawpt-WIDTH))/2;  /* R */
                    *scanpt++ = *rawpt;                                     /* G */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;          /* B */
                } else {
                    /* first line or right column */
                    *scanpt++ = *(rawpt+WIDTH);     /* R */
                    *scanpt++ = *rawpt;             /* G */
                    *scanpt++ = *(rawpt-1); /* B */
                }
            }
        } else {
            if ( (i % 2) == 0 ) {
                /* G(R) */
                if ( (i < (WIDTH*(HEIGHT-1))) && ((i % WIDTH) > 0) ) {
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;          /* R */
                    *scanpt++ = *rawpt;                                     /* G */
                    *scanpt++ = (*(rawpt+WIDTH)+*(rawpt-WIDTH))/2;  /* B */
                } else {
                    /* bottom line or left column */
                    *scanpt++ = *(rawpt+1);         /* R */
                    *scanpt++ = *rawpt;                     /* G */
                    *scanpt++ = *(rawpt-WIDTH);             /* B */
                }
            } else {
                /* R */
                if ( i < (WIDTH*(HEIGHT-1)) && ((i % WIDTH) < (WIDTH-1)) ) {
                    *scanpt++ = *rawpt;                                     /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1)+
                            *(rawpt-WIDTH)+*(rawpt+WIDTH))/4;      /* G */
                    *scanpt++ = (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+
                            *(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;  /* B */
                } else {
                    /* bottom line or right column */
                    *scanpt++ = *rawpt;                             /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt-WIDTH))/2;      /* G */
                    *scanpt++ = *(rawpt-WIDTH-1);           /* B */
                }
            }
        }
        rawpt++;
    }

}

// SGBRG to RGB24
// for some reason, red and blue needs to be swapped
// at least for  046d:092f Logitech, Inc. QuickCam Express Plus to work
//see: http://www.siliconimaging.com/RGB%20Bayer.htm
//and 4.6 at http://tldp.org/HOWTO/html_single/libdc1394-HOWTO/
static void sgbrg2rgb24(long int WIDTH, long int HEIGHT, unsigned char *src, unsigned char *dst){

    long int i;
    unsigned char *rawpt, *scanpt;
    long int size;

    rawpt = src;
    scanpt = dst;
    size = WIDTH*HEIGHT;

    for ( i = 0; i < size; i++ )
    {
        if ( (i/WIDTH) % 2 == 0 ) //even row
        {
            if ( (i % 2) == 0 ) //even pixel
            {
                if ( (i > WIDTH) && ((i % WIDTH) > 0) )
                {
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;       /* R */
                    *scanpt++ = *(rawpt);                        /* G */
                    *scanpt++ = (*(rawpt-WIDTH) + *(rawpt+WIDTH))/2;      /* B */
                } else
                {
                    /* first line or left column */

                    *scanpt++ = *(rawpt+1);           /* R */
                    *scanpt++ = *(rawpt);             /* G */
                    *scanpt++ =  *(rawpt+WIDTH);      /* B */
                }
            } else //odd pixel
            {
                if ( (i > WIDTH) && ((i % WIDTH) < (WIDTH-1)) )
                {
                    *scanpt++ = *(rawpt);       /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1)+*(rawpt-WIDTH)+*(rawpt+WIDTH))/4; /* G */
                    *scanpt++ = (*(rawpt-WIDTH-1) + *(rawpt-WIDTH+1) + *(rawpt+WIDTH-1) + *(rawpt+WIDTH+1))/4;      /* B */
                } else
                {
                    /* first line or right column */

                    *scanpt++ = *(rawpt);       /* R */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+WIDTH))/2; /* G */
                    *scanpt++ = *(rawpt+WIDTH-1);      /* B */
                }
            }
        } else
        { //odd row
            if ( (i % 2) == 0 ) //even pixel
            {
                if ( (i < (WIDTH*(HEIGHT-1))) && ((i % WIDTH) > 0) )
                {
                    *scanpt++ =  (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+*(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;          /* R */
                    *scanpt++ =  (*(rawpt-1)+*(rawpt+1)+*(rawpt-WIDTH)+*(rawpt+WIDTH))/4;      /* G */
                    *scanpt++ =  *(rawpt); /* B */
                } else
                {
                    /* bottom line or left column */

                    *scanpt++ =  *(rawpt-WIDTH+1);          /* R */
                    *scanpt++ =  (*(rawpt+1)+*(rawpt-WIDTH))/2;      /* G */
                    *scanpt++ =  *(rawpt); /* B */
                }
            } else
            { //odd pixel
                if ( i < (WIDTH*(HEIGHT-1)) && ((i % WIDTH) < (WIDTH-1)) )
                {
                    *scanpt++ = (*(rawpt-WIDTH)+*(rawpt+WIDTH))/2;  /* R */
                    *scanpt++ = *(rawpt);      /* G */
                    *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2; /* B */
                } else
                {
                    /* bottom line or right column */

                    *scanpt++ = (*(rawpt-WIDTH));  /* R */
                    *scanpt++ = *(rawpt);      /* G */
                    *scanpt++ = (*(rawpt-1)); /* B */
                }
            }
        }
        rawpt++;
    }
}




#define CLAMP(x)        ((x)<0?0:((x)>255)?255:(x))

typedef struct {
    int is_abs;
    int len;
    int val;
} code_table_t;


/* local storage */
static code_table_t table[256];
static int init_done = 0;

/*
  sonix_decompress_init
  =====================
    pre-calculates a locally stored table for efficient huffman-decoding.
  Each entry at index x in the table represents the codeword
  present at the MSB of byte x.
 */
static void sonix_decompress_init(void){

    int i;
    int is_abs, val, len;

    for (i = 0; i < 256; i++) {
        is_abs = 0;
        val = 0;
        len = 0;
        if ((i & 0x80) == 0) {
            /* code 0 */
            val = 0;
            len = 1;
        }
        else if ((i & 0xE0) == 0x80) {
            /* code 100 */
            val = +4;
            len = 3;
        }
        else if ((i & 0xE0) == 0xA0) {
            /* code 101 */
            val = -4;
            len = 3;
        }
        else if ((i & 0xF0) == 0xD0) {
            /* code 1101 */
            val = +11;
            len = 4;
        }
        else if ((i & 0xF0) == 0xF0) {
            /* code 1111 */
            val = -11;
            len = 4;
        }
        else if ((i & 0xF8) == 0xC8) {
            /* code 11001 */
            val = +20;
            len = 5;
        }
        else if ((i & 0xFC) == 0xC0) {
            /* code 110000 */
            val = -20;
            len = 6;
        }
        else if ((i & 0xFC) == 0xC4) {
            /* code 110001xx: unknown */
            val = 0;
            len = 8;
        }
        else if ((i & 0xF0) == 0xE0) {
            /* code 1110xxxx */
            is_abs = 1;
            val = (i & 0x0F) << 4;
            len = 8;
        }
        table[i].is_abs = is_abs;
        table[i].val = val;
        table[i].len = len;
    }

    init_done = 1;
}

/*
  sonix_decompress
  ================
    decompresses an image encoded by a SN9C101 camera controller chip.
  IN    width
    height
    inp         pointer to compressed frame (with header already stripped)
  OUT   outp    pointer to decompressed frame
  Returns 0 if the operation was successful.
  Returns <0 if operation failed.
 */
static int sonix_decompress(int width, int height, unsigned char *inp, unsigned char *outp)
{
    int row, col;
    int val;
    int bitpos;
    unsigned char code;
    unsigned char *addr;

    if (!init_done) {
        /* do sonix_decompress_init first! */
        ///return -1;
        sonix_decompress_init();
    }

    bitpos = 0;
    for (row = 0; row < height; row++) {

        col = 0;



        /* first two pixels in first two rows are stored as raw 8-bit */
        if (row < 2) {
            addr = inp + (bitpos >> 3);
            code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));
            bitpos += 8;
            *outp++ = code;

            addr = inp + (bitpos >> 3);
            code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));
            bitpos += 8;
            *outp++ = code;

            col += 2;
        }

        while (col < width) {
            /* get bitcode from bitstream */
            addr = inp + (bitpos >> 3);
            code = (addr[0] << (bitpos & 7)) | (addr[1] >> (8 - (bitpos & 7)));

            /* update bit position */
            bitpos += table[code].len;

            /* calculate pixel value */
            val = table[code].val;
            if (!table[code].is_abs) {
                /* value is relative to top and left pixel */
                if (col < 2) {
                    /* left column: relative to top pixel */
                    val += outp[-2*width];
                }
                else if (row < 2) {
                    /* top row: relative to left pixel */
                    val += outp[-2];
                }
                else {
                    /* main area: average of left pixel and top pixel */
                    val += (outp[-2] + outp[-2*width]) / 2;
                }
            }

            /* store pixel */
            *outp++ = CLAMP(val);
            col++;
        }
    }

    return 0;
}


void convertToRgb(viod &vd, Mat &frame){
///void convertToRgb(const Buffer &currentBuffer){

    /// Verificar uma forma melhor depois
    vbuff currentBuffer;
    currentBuffer.start = vd.buffers[vd.bon].start;
    currentBuffer.length = vd.buffers[vd.bon].length;
    currentBuffer.binf = vd.buffers[vd.bon].binf;
    /// ******************************************



    cv::Size imageSize(vd.w, vd.h);
    // Not found conversion
    switch (vd.pxfmt){

        case V4L2_PIX_FMT_YUV411P:
            yuv411p_to_rgb24(imageSize.width, imageSize.height, (unsigned char*)(currentBuffer.start), (unsigned char*)frame.data);
            return;

        case V4L2_PIX_FMT_SBGGR8:
            bayer2rgb24(imageSize.width, imageSize.height, (unsigned char*)currentBuffer.start, (unsigned char*)frame.data);
            return;

        case V4L2_PIX_FMT_SN9C10X:
            ///sonix_decompress_init();
            sonix_decompress(imageSize.width, imageSize.height, (unsigned char*)currentBuffer.start, vd.xbuf);
            bayer2rgb24(imageSize.width, imageSize.height, vd.xbuf, (unsigned char*)frame.data);

            return;

        case V4L2_PIX_FMT_SGBRG8:
            sgbrg2rgb24(imageSize.width, imageSize.height, (unsigned char*)currentBuffer.start, (unsigned char*)frame.data);
            return;

        case V4L2_PIX_FMT_M420:
            ///cv::cvtColor(cv::Mat(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start), destination, COLOR_YUV2BGR_IYUV);
            m420_to_rgb24(imageSize.width, imageSize.height, (unsigned char*)(currentBuffer.start), (unsigned char*)frame.data);
            return;
        default:
            break;

    }
    // Converted by cvtColor or imdecode
    cv::Mat destination(imageSize, CV_8UC3, frame.data);
    switch (vd.pxfmt){

        case V4L2_PIX_FMT_YVU420:
            cv::cvtColor(cv::Mat(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start), destination, COLOR_YUV2BGR_YV12);
            return;

        case V4L2_PIX_FMT_YUV420:
            cv::cvtColor(cv::Mat(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start), destination, COLOR_YUV2BGR_IYUV);
            return;

        case V4L2_PIX_FMT_NV12:
            cv::cvtColor(cv::Mat(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start), destination, COLOR_YUV2RGB_NV12);
            return;

        case V4L2_PIX_FMT_NV21:
            cv::cvtColor(cv::Mat(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start), destination, COLOR_YUV2RGB_NV21);
            return;

///    #ifdef HAVE_JPEG
        case V4L2_PIX_FMT_MJPEG:
        case V4L2_PIX_FMT_JPEG:
        {
            /// Must include IMREAD_IGNORE_ORIENTATION flag
            imdecode(Mat(1, currentBuffer.binf.bytesused, CV_8UC1, currentBuffer.start), IMREAD_COLOR+IMREAD_IGNORE_ORIENTATION, &destination);
            return;
        }
///    #endif

        case V4L2_PIX_FMT_YUYV:
            cvtColor(cv::Mat(imageSize, CV_8UC2, currentBuffer.start), destination, COLOR_YUV2BGR_YUYV);
            return;

        case V4L2_PIX_FMT_UYVY:
            cvtColor(cv::Mat(imageSize, CV_8UC2, currentBuffer.start), destination, COLOR_YUV2BGR_UYVY);
            return;

        case V4L2_PIX_FMT_RGB24:
            cvtColor(cv::Mat(imageSize, CV_8UC3, currentBuffer.start), destination, COLOR_RGB2BGR);
            return;

        case V4L2_PIX_FMT_Y16:
        {
            Mat temp(imageSize, CV_8UC1, vd.xbuf);
            Mat(imageSize, CV_16UC1, currentBuffer.start).convertTo(temp, CV_8U, 1.0 / 256);
            cvtColor(temp, destination, COLOR_GRAY2BGR);
            return;
        }
        case V4L2_PIX_FMT_Y10:
        {
            cv::Mat temp(imageSize, CV_8UC1, vd.xbuf);
            cv::Mat(imageSize, CV_16UC1, currentBuffer.start).convertTo(temp, CV_8U, 1.0 / 4);
            cv::cvtColor(temp, destination, COLOR_GRAY2BGR);
            return;
        }
        case V4L2_PIX_FMT_GREY:
            cv::cvtColor(cv::Mat(imageSize, CV_8UC1, currentBuffer.start), destination, COLOR_GRAY2BGR);
            break;
        case V4L2_PIX_FMT_H264:
        {
            ///Mat tp(1, currentBuffer.binf.bytesused, CV_8U, currentBuffer.start);
            Mat tp(imageSize.height * 3 / 2, imageSize.width, CV_8U, currentBuffer.start);

            cv::cvtColor(tp, destination, CV_YUV2BGR_I420);
            break;
        }
        case V4L2_PIX_FMT_BGR24:
        default:
            memcpy((char *)frame.data, (char *)currentBuffer.start, currentBuffer.binf.bytesused);
            break;

    }
}




unsigned char* data_offset(void* p, int ih, int iw, float rzH, float rzW, int i, int j, int pallete){

    switch(pallete){
        case V4L2_PIX_FMT_YUV411P:
            break;

        case V4L2_PIX_FMT_SBGGR8:
            break;

        case V4L2_PIX_FMT_SN9C10X:
            break;

        case V4L2_PIX_FMT_SGBRG8:
            break;

        case V4L2_PIX_FMT_M420:
            return (unsigned char*)( (unsigned char*)p + int(i * rzH) * iw + int(j * rzW));

        case V4L2_PIX_FMT_YVU420:
            break;

        case V4L2_PIX_FMT_YUV420:
            break;

        case V4L2_PIX_FMT_NV12:
            break;

        case V4L2_PIX_FMT_NV21:
            break;

        case V4L2_PIX_FMT_MJPEG:
            break;

        case V4L2_PIX_FMT_JPEG:
            break;

        case V4L2_PIX_FMT_YUYV:
            return (unsigned char*)( (short int*)p + int(i * rzH) * iw + int(j * rzW));

        case V4L2_PIX_FMT_UYVY:
        {
            unsigned char* q = (unsigned char*)( (short int*)p + int(i * rzH) * iw + int(j * rzW));
            q++;
            return q;
        }

        case V4L2_PIX_FMT_RGB24:
        {
            unsigned char* q = (unsigned char*)( (unsigned char*)p + int(i * rzH) * 3 * iw + 3 * int(j * rzW));
            q++;
            return q;
        }
            break;

        case V4L2_PIX_FMT_Y16:
            break;

        case V4L2_PIX_FMT_Y10:
            break;

        case V4L2_PIX_FMT_GREY:
            return (unsigned char*)( (unsigned char*)p + int(i * rzH) * iw + int(j * rzW));

        case V4L2_PIX_FMT_H264:
            break;

        case V4L2_PIX_FMT_BGR24:
        {
            unsigned char* q = (unsigned char*)( (unsigned char*)p + int(i * rzH) * 3 * iw + 3 * int(j * rzW));
            q++;
            return q;
        }
            break;

    }

    printf("Offset error\n");

    return (unsigned char*)0;

}

void cam_asciiart(char* d_dest, int mTH, int mTW, const void *d_src, int ih, int iw, int pallete){

/// ' .,;!vlLFE$'
/// '$EFLlv!;,. '
/// ' .:-=+*#%@'
/// '@%#*+=-:. '
/// '$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,"^`'. '
/// '@MBHENR#KWXDFPQASUZbdehx*8Gm&04LOVYkpq5Tagns69owz$CIu23Jcfry%1v7l+it[] {}?j|()=~!-/<>\"^_';,:`. '
/// unsigned char map[] = "@MBHENR#KWXDFPQASUZbdehx*8Gm&04LOVYkpq5Tagns69owz$CIu23Jcfry%1v7l+it[]{}?j|()=~!-/<>\"^_';,:`.                                                                                                                                                                   ";
/// unsigned char map[] = "@MBHENR#KWXDFPQASUZbdehx*8Gm&04LOVYkpq5Tagns69owz$CIu23Jcfry81v7l+it[]{}?j|()=~!-i<>i^_';,:`....................................................................................................................................................................";
/// unsigned char map[] = "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM";
    unsigned char mapchar[] = "                               .........................-------------------------:::::::::::::::::::::::::=========================+++++++++++++++++++++++++*************************#########################$$$$$$$$$$$$$$$$$$$$$$$$$@@@@@@@@@@@@@@@@@@@@@@@@@";

    char* pc;
    void* p;
    unsigned char* q;
    int i,j, l;
    float rzH;
    float rzW;

    int tt_sz = 0;  ///title size
    int gapleft;
    int gapright;

    #define HDOT 16
    #define WDOT  8
    #define NCH  20

    p = (void*)d_src;
    pc = d_dest;

    tt_sz = strlen(title);

/// Include title
    gapleft = (mTW-tt_sz)/2;
    gapright = mTW-tt_sz-gapleft;

    ///Insert left gap
    for(l=0;l<gapleft;l++){
        pc[0] = ' ';
        pc++;
    }

    ///Insert title
    sprintf(pc,"%s",title);
    pc += tt_sz;

    ///Insert right gap
    for(l=0;l<gapright;l++){
        pc[0] = ' ';
        pc++;
    }

/// Include blank line
    for(l=0;l<mTW;l++){
        pc[0] = ' ';
        pc++;
    }



    int nch = mTH-4; ///NCH;                    /// Available rows
    int ncw = (iw * nch * HDOT)/(ih * WDOT);    /// proportional cols

    if(ncw > mTW-4){
        ncw = mTW-4;
        nch = (ncw * ih * WDOT)/(iw * HDOT);
    }

    rzH = (1.0*ih)/nch;
    rzW = (1.0*iw)/ncw;

    gapleft = (mTW-ncw)/2;
    gapright = mTW-ncw-gapleft;

    for(i=0;i<nch;i++){
        ///Insert left gap
        for(l=0;l<gapleft;l++){
            pc[0] = ' ';
            pc++;
        }
        ///Insert data
        for(j=0;j<ncw;j++){
            q = data_offset(p,ih,iw,rzH,rzW,i,j,pallete);
            pc[0] = mapchar[q[0]];
            pc++;
        }
        ///Insert right gap
        for(l=0;l<gapright;l++){
            pc[0] = ' ';
            pc++;
        }
    }

    for(i=0;i<(mTH-2-nch);i++){
        for(l=0;l<(gapleft+ncw+gapright);l++){
            pc[0] = ' ';
            pc++;
        }
    }

    pc[0] = 0;

}





void cam_process_image(viod &vd) {

    __u32 tmconv;
    __u32 tmshow;
    __u32 tmasci;
    __u32 tmproc;
    __u32 tmcalc;

    char ptm[100];
    char pst[20];
    char ptt[] = "FPS:  Convert  Show:     Calc     Process   Ascii    Status:";
    char imx[150000];


    ///register time
    gettimeofday(&vd.tm.t1, NULL);

        /// ***********************************************
        if(vd.view || vd.img_proc != 0){
            convertToRgb(vd,vd.v_mat.at(0));
        }
        /// ***********************************************

    ///register time
    gettimeofday(&vd.tm.t2, NULL);

        /// ***********************************************
        if(vd.img_proc != 0){ /// call process
            (*vd.img_proc)(vd);
        }
        /// ***********************************************

    ///register time
    gettimeofday(&vd.tm.t3, NULL);

        /// ***********************************************
        if(vd.view){
            tmconv = (vd.tm.t2.tv_sec-vd.tm.t1.tv_sec)*1000000 + vd.tm.t2.tv_usec - vd.tm.t1.tv_usec;
            tmproc = (vd.tm.t3.tv_sec-vd.tm.t2.tv_sec)*1000000 + vd.tm.t3.tv_usec - vd.tm.t2.tv_usec;
            tmcalc = (vd.tm.t4.tv_sec-vd.tm.tx.tv_sec)*1000000 + vd.tm.t4.tv_usec - vd.tm.tx.tv_usec;
            tmshow = (vd.tm.t5.tv_sec-vd.tm.t4.tv_sec)*1000000 + vd.tm.t5.tv_usec - vd.tm.t4.tv_usec;
            tmasci = (vd.tm.t6.tv_sec-vd.tm.t5.tv_sec)*1000000 + vd.tm.t6.tv_usec - vd.tm.t5.tv_usec;

            tmconv = tmconv<100000?tmconv:0;
            tmshow = tmshow<100000?tmshow:0;
            tmcalc = tmcalc<100000?tmcalc:0;
            tmproc = tmproc<1000000?tmproc:0;
            tmasci = tmasci<100000?tmasci:0;
        }
        /// Update frame rate
        vd.tm.cnt++;
        if((vd.tm.t1.tv_sec*1000 + vd.tm.t1.tv_usec/1000) - (vd.tm.t0.tv_sec*1000 + vd.tm.t0.tv_usec/1000) > 1000){
            vd.tm.fr = vd.tm.cnt - 1;
            vd.tm.cnt = 0;
            vd.tm.t0 = vd.tm.t1;
        }
        /// ***********************************************

        /// store t3 for next time
        vd.tm.tx = vd.tm.t3;

    ///register time
    gettimeofday(&vd.tm.t4, NULL);

        /// ***********************************************
        if(is_there_gui && vd.view){
            if(vd.nv < (int)vd.v_mat.size()){
                ///imshow( "Images", vd.v_mat.at(vd.nv));
                imshow( vd.dvnm, vd.v_mat.at(vd.nv));
            }
            waitKey(1);
        }
        /// ***********************************************

    ///register time
    gettimeofday(&vd.tm.t5, NULL);


        /// ***********************************************
        if(vd.view){
            update_terminal_size();

            /*
            switch(vd.pxfmt){

                case V4L2_PIX_FMT_H264:
                case V4L2_PIX_FMT_MJPEG:
                case V4L2_PIX_FMT_JPEG:
                case V4L2_PIX_FMT_SN9C10X:
                case V4L2_PIX_FMT_SBGGR8:
                    cam_asciiart(imx, maxTH, maxTW, vd.v_mat.at(0).data,vd.h, vd.w, V4L2_PIX_FMT_BGR24);
                    break;
                default:
                    cam_asciiart(imx, maxTH, maxTW, vd.buffers[vd.bon].start,vd.h, vd.w, vd.pxfmt);
                    break;

            }
            */

            if(vd.v_mat.at(vd.nv).type() == CV_8UC1){
                cam_asciiart(imx, maxTH, maxTW, vd.v_mat.at(vd.nv).data,vd.h, vd.w, V4L2_PIX_FMT_GREY);
            }

            if(vd.v_mat.at(vd.nv).type() == CV_8UC3){
                cam_asciiart(imx, maxTH, maxTW, vd.v_mat.at(vd.nv).data,vd.h, vd.w, V4L2_PIX_FMT_BGR24);
            }


            if(vd.st == DEV_CAPTURING){
                sprintf(pst,"Capturing");
            }
            if(vd.st == DEV_PROCESSING){
                sprintf(pst,"Processing %03d", vd.procidx);   /// It seems good.
            }
            sprintf(ptm, "%04d  %05uus  %06uus  %05uus  %06uus  %05uus  %s", vd.tm.fr, tmconv, tmshow, tmcalc, tmproc, tmasci, pst);

            if(wdw.ws_col >= strlen(ptm)){
                GOTOXY(1,1);
                printf("%s",imx);
                fflush(stdout);
                GOTOXY(maxTH-1,1);
                printf("%s",ptt);
                GOTOXY(maxTH,1);
                printf("%s",ptm);
                fflush(stdout);
            }else{

                SCRCLR();
                fflush(stdout);

            }


        }
        /// ***********************************************

    ///register time
    gettimeofday(&vd.tm.t6, NULL);

}

void show_title(){

    SCRCLR();
    printf("\n\n\t\t***********************************\n\n");
    printf("\t\tIoT - Image On Terminal - Multiview");
    printf("\n\n\t\t***********************************\n");

}

void show_help(){

    SCRCLR();
    printf("\n Options for Help\n");
    printf("\n\n");
    printf("\n(O)pen - Show available video device list to open");
    printf("\n(F)eatures - Show available video device list to open");
    printf("\n(G)et - Show available video device list to see settings");
    printf("\n(S)et - Show available video device list to change settings");
    printf("\n(C)apture - Show available video device list to start capture");
    printf("\n(P)rocess - Show available video device list to start process");
    printf("\n(V)iew - Show available video device list to view images captured");
    printf("\n(E)xit - Show available video device list to stop capture and process");
    printf("\n(T)itle - Show title");
    printf("\n(H)elp - Show this help");
    printf("\n(Q)uit - exit program");
    printf("\n\n\n\n");

}

void show_stats(vector<viod*> &vv){

    char p[5] = {0,0,0,0,0};
    char proc[14];
    int dist;
    SCRCLR();

    update_terminal_size();
    dist = ((maxTW-strlen(title))/2) + 1;
    if(dist > (int)strlen(date) + 2){    /// http://man7.org/linux/man-pages/man3/strftime.3.html
                                /// https://www.tutorialspoint.com/c_standard_library/c_function_strftime.htm
        /*
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char s[64];
        assert(strftime(s, sizeof(s), "%c", tm));
        printf("%s\n", s);
        */
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        strftime(date, sizeof(date), "%d/%m/%y", tm);
        printf("%s", date);
        strftime(hour, sizeof(hour), "%X", tm);
        GOTOXY(1,maxTW-7);
        printf("%s", hour);
    }
    GOTOXY(1,dist);
    printf("%s",title);
    printf("\n");
    vector<viod*>::iterator it;
    it = vv.begin();
    printf("\n");
    printf("Pos Device:      FPS: H    W    Fmt: Status\n");
    while(it != vv.end()){

        *((int*)p) = (*it)->pxfmt;
        proc[0] = 0;
        if((*it)->st == DEV_PROCESSING){
            sprintf(proc, ": process %03d", (*it)->procidx);
        }
        printf("%03d /dev/video%d  %04d %04d %04d %s %s%s\n", (int)distance(vv.begin(),it), (*it)->vid.num, (*it)->tm.fr, (*it)->h, (*it)->w, p, cam_status[(*it)->st], proc);
        it++;
    }

    ///printf("  %d: %s: %s, %s, %s\n", i, prefixes[(*it)->vid.typ], name, vcap.card, cam_status[(*it)->st]);

}

// Lê um número do teclado limitado ao valor de cc
// it reads a number from keyboard limited to cc value
int read_dev_num(int cc){

    char c;
    int i;
    int x;
    int cnt;

    cnt = 0;
    x = cc;
    while(x){
        cnt++;
        x = x/10;
    }

    while(1){
        x = 0;
        for(i=0;i<cnt;i++){
            c = getch();
            if(c >= '0' && c <= '9'){
                x = (x * 10) + (c - 48);
            }else{
                break;
            }
        }

        if(i == 0){
            return -1;
        }

        if(x < cc){
            return x;
        }

    }

}

void wait_press_to_continue(){

    GOTOXY(maxTH-1,1);
    printf("\nPress to continue ...");
    fflush(stdout);
    getch();
    SCRCLR();

}


int main(int argc, char **argv){

    char ctl[MAX_CHAR_SEQ];
    int idx;
    int vw;
    int t = 0;
    int ci = 0;
    vector<viod*> v_viod;

    int sz_vet_func = sizeof(vet_of_funcs)/sizeof(TypeFuncWhPar*);

    CLEAR(ctl);

    set_echo(0);

    init_terminal();

    make_camera_list(v_viod);


    printf("\e[?25l"); /// Hide the cursor


    is_there_gui = is_gui_present();
    run_from_cl = from_command_line();

    if(is_there_gui){

        ///fill_lookuptables();q
        namedWindow("Images",1);

    }

    while (1){
        ci = -1;
        while(_kbhit()){
            ci++;
            ctl[ci] = getch();
            if(ci==(MAX_CHAR_SEQ-1)){
                break;
            }
        }


        gettimeofday(&t2, NULL);
        /// one event every second
        if((t2.tv_sec*1000 + t2.tv_usec/1000) - (t0.tv_sec*1000 + t0.tv_usec/1000) > 1000){
            make_camera_list(v_viod);
            t0 = t2;
        }

        if(!is_displaying(v_viod)){
            /// one event every 100 milliseconds
            if((t2.tv_sec*1000 + t2.tv_usec/1000) - (t1.tv_sec*1000 + t1.tv_usec/1000) > 100){
                show_stats(v_viod);
                t1 = t2;
            }

        }


        switch(ctl[0]){
            case 'o':
            case 'O':
                stop_view(v_viod);
                make_camera_list(v_viod);
                show_camera_list(v_viod, DEV_ALL);
                wait_press_to_continue();
                CLEAR(ctl);
                break;

            case 'f':
            case 'F':
                stop_view(v_viod);
                make_camera_list(v_viod);
                t = show_camera_list(v_viod, DEV_ALL);
                if(!t){
                    printf("There is no device to show\n");
                }
                if(v_viod.size() && t){
                    idx = read_dev_num(v_viod.size());
                    if(idx >= 0){
                        SCRCLR();
                        list_v4l2_capabilities(v_viod.at(idx)->fid);
                        wait_press_to_continue();
                        list_video_io(v_viod.at(idx)->fid);
                        wait_press_to_continue();
                        enumerate_controls(v_viod.at(idx)->fid);
                        wait_press_to_continue();
                    }
                }
                //wait_press_to_continue();
                CLEAR(ctl);
                break;

            case 'g':
            case 'G':
                stop_view(v_viod);
                make_camera_list(v_viod);
                t = show_camera_list(v_viod, DEV_ALL);
                if(!t){
                    printf("There is no device to show\n");
                }
                if(v_viod.size() && t){
                    idx = read_dev_num(v_viod.size());
                    if(idx >= 0){
                        SCRCLR();
                        show_video_format(v_viod.at(idx)->fid);
                    }
                }
                wait_press_to_continue();
                CLEAR(ctl);
                break;
            case 's':
            case 'S':
                stop_view(v_viod);
                make_camera_list(v_viod);
                t = show_camera_list(v_viod, DEV_OPEN+DEV_CONFIGURED+DEV_OUR);
                if(!t){
                    printf("There is no device to show\n");
                }
                if(v_viod.size() && t){
                    idx = read_dev_num(v_viod.size());
                    if(idx >= 0){
                        set_echo(1);
                        printf("\e[?25h"); /// show the cursor
                            setting_camera_features(*v_viod.at(idx));
                        printf("\e[?25l"); /// Hide the cursor
                        set_echo(0);
                    }
                }
                CLEAR(ctl);
                break;

            case 'c':
            case 'C':
                stop_view(v_viod);
                make_camera_list(v_viod);
                t = show_camera_list(v_viod, DEV_CONFIGURED+DEV_OUR);
                if(!t){
                    printf("There is no device to show\n");
                }
                if(v_viod.size() && t){
                    idx = read_dev_num(v_viod.size());
                    if(idx >= 0){
                        v_viod.at(idx)->procidx = sz_vet_func-1;
                        pthread_create(&(v_viod.at(idx)->tid), NULL, cam_thread_v4l2, (void *)v_viod.at(idx));
                    }
                }
                CLEAR(ctl);
                break;

            case 'p':
            case 'P':
                vw = stop_view(v_viod);
                make_camera_list(v_viod);
                t = show_camera_list(v_viod, DEV_CAPTURING+DEV_PROCESSING);
                if(!t){
                    printf("There is no device to show\n");
                }
                if(v_viod.size() && t){
                    idx = read_dev_num(v_viod.size());
                    if(idx >= 0){

                        /// Show process options
                        SCRCLR();
                        printf("\n\nChoose the process:\n");
                        int i;
                        for(i=0;i<sz_vet_func-1;i++){

                            printf("\t%d: process00%d\n", i, i);

                        }
                        printf("\tNone - Any key\n");

                        /// Choose a process
                        i = read_dev_num(sz_vet_func);


                        if(v_viod.at(idx)->procidx != i){

                            /// Select firt image to display
                            v_viod.at(idx)->nv = 0;

                            /// Reset initialization
                            v_viod.at(idx)->procinit = 0;

                            if(i == -1){
                                v_viod.at(idx)->procidx = sz_vet_func-1;
                                v_viod.at(idx)->st = DEV_CAPTURING;
                            }else{
                                v_viod.at(idx)->procidx = i;
                                v_viod.at(idx)->st = DEV_PROCESSING;
                            }

                            /// Set new process
                            v_viod.at(idx)->img_proc = vet_of_funcs[v_viod.at(idx)->procidx];

                        }


                        /// Turn view on again
                        if(vw != -1)
                            v_viod.at(vw)->view = 1;


                    }

                }
                CLEAR(ctl);
                break;

            case 'v':
            case 'V':
                stop_view(v_viod);
                make_camera_list(v_viod);
                t = show_camera_list(v_viod, DEV_CAPTURING+DEV_PROCESSING);
                if(!t){
                    printf("There is no device to show\n");
                }
                if(v_viod.size() && t){
                    idx = read_dev_num(v_viod.size());
                    if(idx >= 0){
                        SCRCLR();
                        v_viod.at(idx)->view = 1;
                    }

                }
                CLEAR(ctl);
                break;

            case 'e':
            case 'E':
                stop_view(v_viod);
                make_camera_list(v_viod);
                t = show_camera_list(v_viod, DEV_CAPTURING + DEV_PROCESSING);
                if(!t){
                    printf("There is no device to show\n");
                }
                if(v_viod.size() && t){
                    idx = read_dev_num(v_viod.size());
                    if(idx >= 0){
                        v_viod.at(idx)->thon = 0;
                    }
                }
                CLEAR(ctl);
                break;

            case 't':
            case 'T':
                stop_view(v_viod);
                show_title();
                wait_press_to_continue();
                CLEAR(ctl);
                break;

            case 'h':
            case 'H':
                stop_view(v_viod);
                show_help();
                wait_press_to_continue();
                CLEAR(ctl);
                break;

            case 'q':
            case 'Q':
                int counter;
                stop_view(v_viod);
                SCRCLR();
                printf("Do you really want to exit? [y/n]: ");
                ctl[0] = getch();
                if(ctl[0] == 'y' || ctl[0] == 'Y'){

                    stop_all_threads(v_viod);

                    vector<viod*>::iterator it;
                    it = v_viod.begin();
                    printf("\n");
                    while(it != v_viod.end()){
                        counter = 0;
                        while( (*it)->st == DEV_CAPTURING || (*it)->st == DEV_PROCESSING){

                            /// provavelmente um loop sem conteúdo cria uma rotina de
                            /// de verificação na memória cache que não é atualizada.
                            /// Então, mesmo a thread já tendo sido encerrada o valor
                            /// do status não foi atualizado no cache, apenas na memória principal.
                            /// Assim, o programa fica preso neste loop.
                            /// fazer flush das variáveis. procurar sobre isso.
                            printf("waiting ... %d\r", counter++);

                        }
                        printf("\n");
                        viod* vf = (*it);
                        close_v4l2_device((*vf).fid);
                        printf("Finishing process %u - device: /dev/video%d - File id: %d\n", (unsigned int)(*vf).tid, (*vf).vid.num, (*vf).fid);
                        it++;
                    }

                    printf("\n");
                    set_echo(1);
                    printf("\e[?25h"); /// show the cursor
                    exit(0);
                    break;
                }
                if(ctl[0] == 'n' || ctl[0] == 'N'){
                    ctl[0] = 0;
                    break;
                }
                ///CLEAR(ctl);
                printf("\n");
                break;

            case 27:

                SCRCLR();
                make_camera_list(v_viod);

                if(!strcmp(&ctl[1],"[A")){   /// up arrow    choose next camera

                    int d = stop_view(v_viod);
                    if(d != -1){
                        for(int i=0;i<(int)v_viod.size();i++){
                            d++;
                            if(d >= (int)v_viod.size()){
                                d = 0;
                            }
                            if(v_viod.at(d)->st == DEV_CAPTURING || v_viod.at(d)->st == DEV_PROCESSING){
                                v_viod.at(d)->view = 1;
                                break;
                            }
                        }
                    }

                }

                if(!strcmp(&ctl[1],"[B")){   /// down arrow    choose previous camera

                    int d = stop_view(v_viod);
                    if(d != -1){
                        for(int i=0;i<(int)v_viod.size();i++){
                            d--;
                            if(d < 0){
                                d += v_viod.size();
                            }
                            if(v_viod.at(d)->st == DEV_CAPTURING || v_viod.at(d)->st == DEV_PROCESSING){
                                v_viod.at(d)->view = 1;
                                break;
                            }
                        }
                    }

                }

                if(!strcmp(&ctl[1],"[C")){   /// right arrow - Choose next image

                    vector<viod*>::iterator it;
                    it = v_viod.begin();
                    while(it != v_viod.end()){
                        if( (*it)->view){
                            (*it)->nv++;
                            if((*it)->nv >= (int)(*it)->v_mat.size()){
                                (*it)->nv = 0;
                            }
                            break;
                        }
                        it++;
                    }

                }

                if(!strcmp(&ctl[1],"[D")){   /// left arrow - Choose previous image

                    vector<viod*>::iterator it;
                    it = v_viod.begin();
                    while(it != v_viod.end()){
                        if((*it)->view){
                            (*it)->nv--;
                            if((*it)->nv < 0){
                                (*it)->nv += (*it)->v_mat.size();
                            }
                            break;
                        }
                        it++;
                    }

                }
                if(!strcmp(&ctl[1],"[H")){   /// Home
                }
                if(!strcmp(&ctl[1],"[E")){   /// End
                }
                if(!strcmp(&ctl[1],"[5~")){  /// PgUp    it Chooses next process

                    /// Search of displayed camera
                    vector<viod*>::iterator it;
                    it = v_viod.begin();
                    while(it != v_viod.end()){
                        if( (*it)->view){

                            /// Select firt image to display
                            (*it)->nv = 0;

                            /// stop the current process
                            (*it)->img_proc = 0;

                            /// deve resetar a inicialização
                            (*it)->procinit = 0;

                            (*it)->procidx++;
                            if((*it)->procidx >= sz_vet_func){
                                (*it)->procidx = 0;
                            }

                            (*it)->img_proc = vet_of_funcs[(*it)->procidx];

                            (*it)->st = (*it)->img_proc ? DEV_PROCESSING : DEV_CAPTURING;

                            break;
                        }
                        it++;
                    }

                }

                if(!strcmp(&ctl[1],"[6~")){  /// PgDn    it chooses previous process

                    /// Search of displayed camera
                    vector<viod*>::iterator it;
                    it = v_viod.begin();
                    while(it != v_viod.end()){
                        if( (*it)->view){

                            /// Select firt image to display
                            (*it)->nv = 0;
                            /// stop the current process
                            (*it)->img_proc = 0;

                            /// deve resetar a inicialização
                            (*it)->procinit = 0;

                            (*it)->procidx--;
                            if((*it)->procidx < 0){
                                (*it)->procidx += sz_vet_func;
                            }

                            (*it)->img_proc = vet_of_funcs[(*it)->procidx];

                            (*it)->st = (*it)->img_proc ? DEV_PROCESSING : DEV_CAPTURING;

                            break;
                        }
                        it++;
                    }

                }

                if(!strcmp(&ctl[1],"[2~")){  /// Insert
                }
                if(!strcmp(&ctl[1],"[3~")){  /// Delete
                }
                if(!strcmp(&ctl[1],"OP")){   /// F1
                }
                if(!strcmp(&ctl[1],"OQ")){   /// F2
                }
                if(!strcmp(&ctl[1],"OR")){   /// F3
                }
                if(!strcmp(&ctl[1],"OS")){   /// F4
                }
                if(!strcmp(&ctl[1],"[15~")){   /// F5
                }
                if(!strcmp(&ctl[1],"[17~")){   /// F6
                }
                if(!strcmp(&ctl[1],"[18~")){   /// F7
                }
                if(!strcmp(&ctl[1],"[19~")){   /// F8
                }
                if(!strcmp(&ctl[1],"[20~")){   /// F9
                }
                if(!strcmp(&ctl[1],"[21~")){   /// F10
                }
                if(!strcmp(&ctl[1],"[23~")){   /// F11
                }
                if(!strcmp(&ctl[1],"[24~")){   /// F12
                }
                if(!strcmp(&ctl[1],"[29~")){   /// Menu list
                }

                CLEAR(ctl);
                break;

        }

    }



    fprintf(stderr, "\n");

    set_echo(1);
    printf("\e[?25h"); /// show the cursor

    return 0;
}
