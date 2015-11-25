/*
  PTG-DLL 
  -------

  Modul:    ptg.h
  Version:  1.001
  Datum:    08.06.2010
  Autor:    F. Kern
  Lizenz:   GNU-Lizenz

  Beschreibung:

  Deklarationsteil/Headerdatei für die DLL-Schnittstelle zum Leica PTG-Format 
 
  v1.000 : erste Veröffentlichung
  v1.001 : PTG__Local2World() zur Umrechnung vom Scannerkoordinatensystem ins globale Koordinatensystem

  fredie.kern@fh-mainz.de
  www.i3mainz.fh-mainz.de
*/
 /************************************************************************
  * Copyright (C) 2010 by Fredie Kern  www.i3mainz.fh-mainz.de           *
  *                                                                      *
  * This program is free software; you can redistribute it and/or modify *
  * it under the terms of the GNU General Public License as published by *
  * the Free Software Foundation; either version 3 of the License, or    *
  * (at your option) any later version.                                  *
  *                                                                      *
  * This program is distributed in the hope that it will be useful,      *
  * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
  * GNU General Public License for more details.                         *
  *                                                                      *
  * You should have received a copy of the GNU General Public License    *
  * along with this program; if not, see <http://www.gnu.org/licenses/>  *
  * or write to the                                                      *
  * Free Software Foundation, Inc.,                                      *
  * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.            *
  *                                                                      *
   **********************************************************************/

//---------------------------------------------------------------------------
#ifndef ptg_H
#define ptg_H
//---------------------------------------------------------------------------

#include <windows.h>


// VC++      benötigt __cdecl   damit keine Namenserweiterung mit nachfolgendem @ erstellt wird
// Borland   benötigt __stdcall damit keine Namenserweiterung mit nachfolgendem @ erstellt wird


#define __nameextension __cdecl
#ifdef __ISBC__
#undef  __nameextension
#define __nameextension __stdcall
#endif


//#define __BUILDING_THE_DLL


#ifdef __BUILDING_THE_DLL

//#define __EXPORT_TYPE __export
#define  __EXPORT_TYPE dllexport
#include "cPTG.h"

#else

//#define __EXPORT_TYPE __import
#define  __EXPORT_TYPE dllimport
#endif

#define VERSION_100

extern "C" char PTG_FILETYPETAG[];
extern "C" long PTG_MAGICNUMBER;
extern "C" int  PTG_PROP_XYZFLOAT;
extern "C" int  PTG_PROP_XYZDOUBLE;
extern "C" int  PTG_PROP_INTENSITY;
extern "C" int  PTG_PROP_RGBVALUE;



typedef  void (*TPTG_echo) (char *str,...);

typedef int TPTG_handle;


#pragma option -a1

#pragma pack(1)
struct _PTG_point
{
  double x[3];             // cartesian 3D coordinates   8 byte  [-1000.0000,1000.0000]  24 byte
  float  I;                // intensity                  4 byte  [0.0,1.0]                4 byte
  unsigned char  rgb[3];   // RGB-color                  1 byte  [0,255]                  3 byte
};
typedef struct _PTG_point TPTG_point;

#pragma option -a8


struct _PTG_metadata_mandatory
{
  long   version;
  long   cols;
  long   rows;
  long   properties;
  double tmatrix[4*4];
/*
        |r11 r12 r13  0|
   T  = |r21 r22 r23  0|
  4x4   |r31 r32 r33  0|
        |t1   t2  t3  1|

        with:
          r(i,j) = elements of rotation matrix
          t(i)   = element of translation vector
          s      = scale

   x_global = x_local * T
*/
};

typedef struct _PTG_metadata_mandatory TPTG_metadata_mandatory;



/*
  Informationen über die PTG-DLL abfragen

  Es ist ein char-Zeiger zu übergeben, der im aufrufenden Programm ausreichend gross
  (z.B. 512 Zeichen) dimensioniert ist.

  Rückgabe:
    der übergebene Zeiger

*/

//extern "C" void* _stdcall tls_io__About(void *pstr);
extern "C" char*          __nameextension PTG__About(char *pstr);


extern "C" void          __nameextension PTG__nogeoreference(double *p);
extern "C" void          __nameextension PTG__copygeoreference(double *p,double *q);

extern "C" void          __nameextension PTG__Init(
                       void (*warnings) (char *str,...),
                       void (*errors)   (char *str,...),
                       int   debuglevel,
                       char *creator=NULL
                     );

extern "C" void           __nameextension  PTG__Exit(void);


extern "C" TPTG_handle    __nameextension PTG__Create(char *fname,TPTG_metadata_mandatory *mdata);
extern "C" void           __nameextension PTG__AddPoint(TPTG_handle handle,TPTG_point *point);

extern "C" TPTG_handle    __nameextension PTG__Open(char *fname,TPTG_metadata_mandatory *mdata);
extern "C" bool           __nameextension PTG__GetPoint(TPTG_handle handle,TPTG_point *point);
/*
    Liefert den an der Position row, column in der Datenmatrix gespeicherten Messpunkt 
*/
extern "C" bool           __nameextension PTG__GetPointColRow(TPTG_handle handle,int col, int row, TPTG_point *point);

extern "C" void           __nameextension PTG__Close(TPTG_handle handle);
/*
   Transformiert die lokalen Koordinaten im Scannerkoordinatensystem ins Welt-Koordinatensystem
*/
extern "C" void           __nameextension PTG__Local2World(TPTG_handle handle,TPTG_point *point,double *X);

#endif
