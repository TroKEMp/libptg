/*
  PTG-DLL 
  -------

  Modul:    ptg.cpp
  Version:  1.001
  Datum:    08.06.2010
  Autor:    F. Kern
  Lizenz:   GNU-Lizenz

  Beschreibung:

  Implementationsteil der DLL-Schnittstelle zum Leica PTG-Format
  Wrapper für die C-Bibliothek cPTG.cpp, cPTG.h
 
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

#include <windows.h>
//---------------------------------------------------------------------------
//   Wichtiger Hinweis zur DLL-Speicherverwaltung, falls die DLL die statische
//   Version der Laufzeitbibliothek (RTL) verwendet:
//
//   Wenn die DLL Funktionen exportiert, die String-Objekte (oder Strukturen/
//   Klassen, die verschachtelte Strings enthalten) als Parameter oder Funktionsergebnisse übergibt,
//   muss die Bibliothek MEMMGR.LIB im DLL-Projekt und anderen Projekten,
//   die die DLL verwenden, vorhanden sein. Sie benötigen MEMMGR.LIB auch dann,
//   wenn andere Projekte, die die DLL verwenden, new- oder delete-Operationen
//   auf Klassen anwenden, die nicht von TObject abgeleitet sind und die aus der DLL exportiert
//   werden. Durch das Hinzufügen von MEMMGR.LIB wird die DLL und deren aufrufende EXEs
//   angewiesen, BORLNDMM.DLL als Speicherverwaltung zu benutzen. In diesem Fall
//   sollte die Datei BORLNDMM.DLL zusammen mit der DLL weitergegeben werden.
//
//   Um die Verwendung von BORLNDMM.DLL, zu vermeiden, sollten String-Informationen als "char *" oder
//   ShortString-Parameter weitergegeben werden.
//
//   Falls die DLL die dynamische Version der RTL verwendet, müssen Sie
//   MEMMGR.LIB nicht explizit angeben.
//---------------------------------------------------------------------------


#include "ptg.h"

#define DLLNAME        "PTG-DLL"
#define DLLFILENAME    "ptg.dll"
#define DLLAUTHOR      "Prof. Dr.-Ing. Fredie Kern"
#define DLLVERSION     "v1.00 (17.02.2010)"
#define DLLCOPYRIGHT   "(c) 2010 i3mainz - Institut für raumbezogene Mess- und Informationssysteme - Fachhochschule Mainz"
#undef DLLVERSION
#define DLLVERSION     "v1.01 (08.06.2010)"

char PTG_FILETYPETAG[]    = FILETYPETAG;
long PTG_MAGICNUMBER    = MAGICNUMBER;    
int  PTG_PROP_XYZFLOAT  = PROP_XYZFLOAT;
int  PTG_PROP_XYZDOUBLE = PROP_XYZDOUBLE;
int  PTG_PROP_INTENSITY = PROP_INTENSITY;
int  PTG_PROP_RGBVALUE  = PROP_RGBVALUE;

static const int    MAX_handle_idx=1;
static int          act_handle_idx=0;
static _cPTG_handle *handles[1]={NULL};

extern "C" char* __nameextension PTG__About(char *pstr)
{
  char str[256]="";
  char compilerstr[256]="";

#if defined(__BORLANDC__)
  sprintf(str,"BC %X",__BORLANDC__);
#elif defined(_MSC_VER)
  sprintf(str,"MSC %X",_MSC_VER);
#endif
  sprintf(compilerstr,"%s",str);
  sprintf((char *) pstr,"%s\n%s\n%s\n%s\n%s\n%s",DLLFILENAME,compilerstr,DLLNAME,DLLVERSION,DLLCOPYRIGHT,DLLAUTHOR);
  return(pstr);
}

extern "C" void          __nameextension PTG__nogeoreference(double *p)
{
  _cPTG_nogeoreference(p);
}

extern "C" void          __nameextension PTG__copygeoreference(double *p,double *q)
{
  _cPTG_copygeoreference(p,q);
}


extern "C" void __nameextension PTG__Init(
                       void (*warnings) (char *str,...),
                       void (*errors)   (char *str,...),
                       int   debuglevel,
                       char *creator
                     )
{
  act_handle_idx = 0;
  _cPTG_init(warnings,errors,debuglevel,creator);
}

extern "C" void __nameextension PTG__Exit(void)
{
   _cPTG_exit();
}

extern "C" TPTG_handle  __nameextension PTG__Create(char *fname,TPTG_metadata_mandatory *mdata)
{
  if (act_handle_idx<MAX_handle_idx)
  {
    handles[act_handle_idx] =   _cPTG_create(fname,(T_cPTG_metadata_mandatory *) mdata);	
	act_handle_idx++;
	return(act_handle_idx-1);
  }
  return(-1);
}

extern "C" void         __nameextension PTG__AddPoint(TPTG_handle handle,TPTG_point *point)
{
 if (point!=NULL)
 { 
   T_cPTG_xyzIrgb pt;

   for (int k=0;k<3;k++) pt.x[k]   = point->x[k];
                         pt.I      = point->I;
   for (int k=0;k<3;k++) pt.rgb[k] = point->rgb[k];
   _cPTG_add_xyzIrgb(handles[handle],&pt);
 }
 else
 {
   _cPTG_add_xyzIrgb(handles[handle],NULL);	  
 }  
}


extern "C" TPTG_handle __nameextension PTG__Open(char *fname,TPTG_metadata_mandatory *mdata)
{
  if (act_handle_idx<MAX_handle_idx)
  {
    handles[act_handle_idx] = _cPTG_open(fname,(T_cPTG_metadata_mandatory *) mdata);	
	act_handle_idx++;
	return(act_handle_idx-1);
  }
  return(-1);
}

extern "C" bool            __nameextension PTG__GetPoint(TPTG_handle handle,TPTG_point *point)
{
 T_cPTG_xyzIrgb pt;
 bool ret;

 ret = _cPTG_get_xyzIrgb(handles[handle],&pt);

 for (int k=0;k<3;k++) point->x[k]   = pt.x[k];
                       point->I      = pt.I;
 for (int k=0;k<3;k++) point->rgb[k] = pt.rgb[k];

 return(ret);
}

/*
    Liefert den an der Position col, row in der Datenmatrix gespeicherten Messpunkt 
*/
extern "C" bool           __nameextension PTG__GetPointColRow(TPTG_handle handle,int col,int row, TPTG_point *point)
{
   T_cPTG_xyzIrgb pt;
   bool ret;

   ret = _cPTG_get_xyzIrgb(handles[handle],col,row,&pt);

   for (int k=0;k<3;k++) point->x[k]   = pt.x[k];
                         point->I      = pt.I;
   for (int k=0;k<3;k++) point->rgb[k] = pt.rgb[k];

   return(ret);
}

extern "C" void           __nameextension PTG__Close(TPTG_handle handle)
{
  if (act_handle_idx>=0)
  {
    _cPTG_close(handles[handle]);
    act_handle_idx--;
  }
}

extern "C" void          __nameextension PTG__Local2World(TPTG_handle handle,TPTG_point *point,double *X)
{
  T_cPTG_xyzIrgb pt;

  for (int k=0;k<3;k++) pt.x[k]   = point->x[k];

	_cPTG_Local2World(handles[handle],&pt,X);
}

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
  act_handle_idx = 0;
  return 1;
}
//---------------------------------------------------------------------------
