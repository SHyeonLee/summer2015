#include "stdafx.h"
#include "stdio.h"

#include "dcmtk/include/dcmtk/config/osconfig.h"

BEGIN_EXTERN_C
#define boolean ijg_boolean
#include "dcmtk/include/jpeglib16.h"
#include "dcmtk/include/jerror16.h"
#undef boolean

#ifdef const
#undef const
#endif
END_EXTERN_C

#include <setjmp.h>
#include <fcntl.h>
#include "dcmtk/include/jpeglib12.h"

struct my_error_mgr16 {
   struct jpeg_error_mgr pub;
   jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr16 * my_error_ptr16;

void my_error_exit16(j_common_ptr cinfo)
{
   my_error_ptr16 myerr = (my_error_ptr16) cinfo->err;
   (*cinfo->err->output_message) (cinfo);
   longjmp(myerr->setjmp_buffer, 1);
}

static void init_source (j_decompress_ptr cinfo)
{

}

static int fill_input_buffer (j_decompress_ptr cinfo) 
{
   ERREXIT(cinfo, JERR_INPUT_EMPTY); 
   return TRUE; 
}

static void skip_input_data(j_decompress_ptr cinfo, long num_bytes) 
{
   struct jpeg_source_mgr* src = (struct jpeg_source_mgr*) cinfo->src;
   if (num_bytes > 0) {
      src->next_input_byte += (size_t) num_bytes;
      src->bytes_in_buffer -= (size_t) num_bytes;
   }
} 

static void term_source (j_decompress_ptr cinfo) 
{

} 

void jpeg_mem_src(j_decompress_ptr cinfo, void* buffer, long nbytes);

void jpeg_mem_src(j_decompress_ptr cinfo, void* buffer, long nbytes)
{
   struct jpeg_source_mgr * src;
   if(cinfo->src == NULL){ /* first time for this JPEG object? */
      cinfo->src = (struct jpeg_source_mgr *) 
         (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,             
         sizeof(struct jpeg_source_mgr));
   }

   src = (struct jpeg_source_mgr*) cinfo->src;
   src->init_source = init_source;
   src->fill_input_buffer = fill_input_buffer;
   src->skip_input_data = skip_input_data;
   src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
   src->term_source = term_source;
   src->bytes_in_buffer = nbytes;
   src->next_input_byte = (JOCTET*)buffer; 
}

BOOL KDicomDS::DecodeJpegMem16(unsigned char * pSrc, unsigned int srcLength, unsigned char * pDst)
{
   struct jpeg_decompress_struct cinfo;
   struct my_error_mgr16 jerr;

   JSAMPARRAY buffer;	// Output row buffer
   int row_stride;		// physical row width in output buffer

   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = my_error_exit16;

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
   row_stride = cinfo.output_width * step * cinfo.output_components;

   buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

   while (cinfo.output_scanline < cinfo.output_height){
      jpeg_read_scanlines(&cinfo, buffer, 1);
      memcpy(pDst + (cinfo.output_scanline - 1) * row_stride, buffer[0], row_stride);
   }

   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);

   return TRUE;
}
