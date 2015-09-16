#include "stdafx.h"
#include "stdio.h"

#include "dcmtk/include/dcmtk/config/osconfig.h"

BEGIN_EXTERN_C
#define boolean ijg_boolean
#include "dcmtk/include/jpeglib8.h"
#include "dcmtk/include/jerror8.h"
#undef boolean

#ifdef const
#undef const
#endif
END_EXTERN_C

#include <setjmp.h>
#include <fcntl.h>

extern void jpeg_mem_src(j_decompress_ptr cinfo, void* buffer, long nbytes);

struct my_error_mgr8 {
   struct jpeg_error_mgr pub;
   jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr8 * my_error_ptr8;

void my_error_exit8(j_common_ptr cinfo)
{
   my_error_ptr8 myerr = (my_error_ptr8) cinfo->err;
   (*cinfo->err->output_message) (cinfo);
   longjmp(myerr->setjmp_buffer, 1);
}

BOOL KDicomDS::DecodeJpegMem8(unsigned char * pSrc, unsigned int srcLength, unsigned char * pDst)
{
   struct jpeg_decompress_struct cinfo;
   struct my_error_mgr8 jerr;

   JSAMPARRAY buffer;	// Output row buffer
   int row_stride;		// physical row width in output buffer

   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit8;

   if (setjmp(jerr.setjmp_buffer)) {
      jpeg_destroy_decompress(&cinfo);
      return FALSE;
   }

   jpeg_create_decompress(&cinfo);

   //   jpeg_stdio_src(&cinfo, file);
   jpeg_mem_src(&cinfo, pSrc, srcLength);

   (void) jpeg_read_header(&cinfo, TRUE);

   // Sample 309
   if(m_nPhotometric == RGB && cinfo.jpeg_color_space == JCS_YCbCr)
      cinfo.jpeg_color_space = JCS_RGB;

   (void) jpeg_start_decompress(&cinfo);

   row_stride = cinfo.output_width * cinfo.output_components;

   buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

   while (cinfo.output_scanline < cinfo.output_height){
      jpeg_read_scanlines(&cinfo, buffer, 1);
      memcpy(pDst + (cinfo.output_scanline - 1) * row_stride, buffer[0], row_stride);
   }

   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);

   return TRUE;
}

