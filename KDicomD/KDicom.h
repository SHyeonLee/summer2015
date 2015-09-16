#pragma once

#ifdef KDICOMDLL
#define KDICOM_CLASS    AFX_EXT_CLASS
#else
#define KDICOM_CLASS    
#endif

#define MAX_CHANNEL						1
#define MAX_OVERLAY_TEXT				20
#define IMPLEMENTATION_CLASS_UID	   _T("1.2.826.0.1.3680043.2.1211.1")

typedef enum{
   KE_CANNOT_CREATE_FILE, KE_CANNOT_CREATE_FILE_MAP, KE_CANNOT_MAP_VIEW_OF_FILE, KE_FILE_LENGTH_IS_ZERO, KE_CANNOT_OPEN_FILE,
   KE_TRY_TO_READ_INVALID_FILE_AREA, KE_IMAGE_PARAMETER_MISSING, KE_CANNOT_WRITE_TO_FILE
} KD_ERROR_CODE;

typedef enum{
	IMPLICIT_LITTLE, EXPLICIT_LITTLE, EXPLICIT_BIG,
	JPEG_BASELINE, JPEG_EXTENDED_2_4, JPEG_EXTENDED_3_5, JPEG_SPECTRAL_6_8, JPEG_SPECTRAL_7_9,
	JPEG_FULL_10_12, JPEG_FULL_11_13, JPEG_LOSSLESS_14, JPEG_LOSSLESS_15, JPEG_EXTENDED_16_18,
	JPEG_EXTENDED_17_19, JPEG_SPECTRAL_20_22, JPEG_SPECTRAL_21_23, JPEG_FULL_24_26,
	JPEG_FULL_25_27, JPEG_LOSSLESS_28, JPEG_LOSSLESS_29, JPEG_LOSSLESS_FIRST_14, RLE,
	JPEG_2000_IMAGE_COMPRESSON_LOSSLESS_ONLY, JPEG_2000_IMAGE_COMPRESSON, UNKNOWN_TRANSFER_SYNTAX, TS_AUTO
} KD_TRANSFER_SYNTAX;

typedef enum{
	IMPLICIT, EXPLICIT
} KD_TRANSFER_VR;

typedef enum {
	AE, AS, AT, CS, DA, DS, DT, FL, FD, IS, LO, LT, OB, OW, PN, SH, SL, SQ, SS, ST, TM, UI, UL, UN, US, UT, NF 
} KD_VR_ENUM;

typedef enum {
	DAY, MONTH, WEEK, YEAR
} KD_AS_ENUM;

typedef enum {
	MONOCHROME1, MONOCHROME2, PALETTE_COLOR, RGB, HSV,
   ARGB, CMYK, YBR_FULL, YBR_FULL_422, YBR_PARTIAL_422, 
   YBR_PARTIAL_420, YBR_ICT, YBR_RCT, USER_PALETTE
} KD_PHOTOMETRIC;

typedef enum {
	POINTER, PAN, ZOOM, MAGNIFY, USER, WINDOW, SHUTTER, BOX_AUTOWINDOW
} KD_LM_ENUM;

typedef enum {
   KZ_IMAGE, KZ_REALSIZE, KZ_FIT
} KD_ZOOM_TYPE;

typedef enum {
   KR_UNCOMPRESSED, KR_JPEG, KR_JPEG2000, KR_RLE
} KD_RAWIMAGE_TYPE;

typedef struct {
	int				num;
	KD_AS_ENUM		type;
} KD_AS;

typedef struct {
	int				year;
	int				month;
	int				day;
} KD_DA;

typedef struct {
	int hour;
	int min;
	int sec;
	int fraction;
} KD_TM;

typedef struct {
	KD_DA			date;
	KD_TM			time;
	int				sign;
	int				offset_hour;
	int				offset_min;
} KD_DT;

typedef struct {
	CString			family;
	CString			given;
	CString			middle;
	CString			prefix;
	CString			suffix;
   CString			family2;
   CString			given2;
   CString			middle2;
   CString			prefix2;
   CString			suffix2;
   CString			family3;
   CString			given3;
   CString			middle3;
   CString			prefix3;
   CString			suffix3;
   BOOL           multi;
} KD_PN;

typedef struct {
	unsigned short group;
	unsigned short element;
} KD_TAG;

typedef struct {
	KD_TAG			tag;
	KD_TAG			mask;
	char			   name[128];
	char			   VR[4];
	int				minVM;
	int				maxVM;
} KD_ATTRIBUTE;

typedef enum {
	MALE, FEMALE, NA
} KD_SEX;

typedef struct{
	int	year;
	int month;
	int day;
} KD_DATE;

typedef enum {
	KD_DICOM, KD_DIB, KD_RAW, KD_JPEG, KD_TIFF
} KD_IMAGE_FORMAT;

typedef enum {
	UNIT_MILLI, UNIT_CENTI, UNIT_PIXEL, UNIT_INCH
} KD_UNIT;

typedef enum {
	OP_LEFT_TOP, OP_CENTER_TOP, OP_CENTER_RIGHT_TOP, OP_CENTER_LEFT_TOP, OP_RIGHT_TOP, 
   OP_LEFT_BOTTOM, OP_CENTER_BOTTOM, OP_CENTER_RIGHT_BOTTOM, OP_CENTER_LEFT_BOTTOM, OP_RIGHT_BOTTOM   
} KD_OVERLAY_POS;

typedef enum {
	KIP_INVERSE, KIP_FLIP_V, KIP_FLIP_H, KIP_ROTATE_CW90, KIP_ROTATE_CCW90, KIP_NONE, KIP_MERGE, 
} KD_IMAGE_PROC;

typedef enum {
   KL_LINEAR, KL_LOG, KL_EXP
} KD_LUT;

#ifndef __AFXTEMPL_H__
	#include <afxtempl.h>
#endif
#ifndef __PROCESS_H__
	#include <process.h>
#endif

#define	PI 3.141592

void KD_W2A(CString value, char * string);
CString KD_A2W(char * value);

#include "math.h"
#include "KDicomElement.h"
#include "KDicomQuery.h"
#include "KDicomDS.h"