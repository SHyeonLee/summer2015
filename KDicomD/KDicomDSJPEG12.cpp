#include "stdafx.h"
#include "stdio.h"

#include "dcmtk/include/dcmtk/config/osconfig.h"

BEGIN_EXTERN_C
#define boolean ijg_boolean
#include "dcmtk/include/jpeglib12.h"
#include "dcmtk/include/jerror12.h"
#undef boolean

#ifdef const
#undef const
#endif
END_EXTERN_C

#include <setjmp.h>
#include <fcntl.h>

extern void jpeg_mem_src(j_decompress_ptr cinfo, void* buffer, long nbytes);

struct my_error_mgr12 {
   struct jpeg_error_mgr pub;
   jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr12 * my_error_ptr12;

void my_error_exit12(j_common_ptr cinfo)
{
   my_error_ptr12 myerr = (my_error_ptr12) cinfo->err;
   (*cinfo->err->output_message) (cinfo);
   longjmp(myerr->setjmp_buffer, 1);
}

BOOL KDicomDS::DecodeJpegMem12(unsigned char * pSrc, unsigned int srcLength, unsigned char * pDst)
{
   struct jpeg_decompress_struct cinfo;
   struct my_error_mgr12 jerr;

   JSAMPARRAY buffer;	// Output row buffer
   int row_stride;		// physical row width in output buffer

   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit12;

   if (setjmp(jerr.setjmp_buffer)) {
      jpeg_destroy_decompress(&cinfo);
      return FALSE;
   }

   jpeg_create_decompress(&cinfo);

   //   jpeg_stdio_src(&cinfo, file);
   jpeg_mem_src(&cinfo, pSrc, srcLength);

   (void) jpeg_read_header(&cinfo, TRUE);

   (void) jpeg_start_decompress(&cinfo);

   int step;
   if(cinfo.data_precision <= 8)
      step = 1;
   else 
      step = 2;
   row_stride = cinfo.output_width * step;

   buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

   while (cinfo.output_scanline < cinfo.output_height){
      jpeg_read_scanlines(&cinfo, buffer, 1);
      memcpy(pDst + (cinfo.output_scanline - 1) * cinfo.output_width * step, buffer[0], row_stride);
   }

   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);

   return TRUE;
}