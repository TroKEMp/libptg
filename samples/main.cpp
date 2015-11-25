/*
  PTG-DLL Test 01
  -------------------

  Modul:    PTG-DLL_test_01_main_main.cpp
  Version:  1.000
  Datum:    17.02.2010
  Autor:    F. Kern

  Beschreibung:

  Hauptprogramm zum Testen von PTG-DLL



  fredie.kern@fh-mainz.de
  www.i3mainz.fh-mainz.de
*/
//---------------------------------------------------------------------------
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

#pragma hdrstop

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "ptg.h"

#define PCOPYRIGHT "(c) 2010 i3mainz, Prof. Dr.-Ing. Fredie Kern"
#define PVERSION   "v1.002"

void Meldung(char *fmt,...)
{
  va_list va;

  va_start(va,fmt);
  vfprintf(stderr,fmt,va);
  va_end(va);
}

void ProtoMeldung(char *fmt,...)
{
  va_list va;

  va_start(va,fmt);
  vfprintf(stderr,fmt,va);
  va_end(va);
}

void Error(char *fmt,...)
{
  va_list va;

  va_start(va,fmt);
  vfprintf(stderr,fmt,va);
  va_end(va);
  exit(-1);
}


void Usage(char *arg)
{
  Meldung(" Programm zur Konvertierung einer Punktwolke im PTG-Format ins PTS-Format.\n");
  Meldung("\n");
  Meldung(" Aufruf: %s <Scandaten *.ptg> <Scandaten *.pts> [Optionen]\n",arg);
  Meldung(" Optionen:\n");
  Meldung("\n");
  Meldung("                        <x> muá, [x] kann angegeben werden. (x) Default\n");
}

void test01()
{
  TPTG_point              point;
  TPTG_handle             ptg;
  TPTG_metadata_mandatory mdata;

  PTG__Init(ProtoMeldung,Error,0,NULL);

  mdata.version = 1;
  mdata.cols = 3;
  mdata.rows = 4;
  mdata.properties = PTG_PROP_XYZFLOAT | PTG_PROP_INTENSITY | PTG_PROP_RGBVALUE;
  PTG__nogeoreference(mdata.tmatrix);

  ptg = PTG__Create("dummy.ptg",&mdata);
  if (ptg!=-1)
  {
    point.x[0] = 11.0; point.x[1] = 10.0; point.x[2] = 10.0; point.I    = 0.6; point.rgb[0] =   0; point.rgb[1] = 128; point.rgb[2] =  67;
    PTG__AddPoint(ptg,&point);
    point.x[0] = 12.0; point.x[1] = 10.0; point.x[2] = 10.0; point.I    = 0.6; point.rgb[0] =   0; point.rgb[1] = 128; point.rgb[2] =  67;
    PTG__AddPoint(ptg,&point);
    point.x[0] = 13.0; point.x[1] = 10.0; point.x[2] = 10.0; point.I    = 0.6; point.rgb[0] =   0; point.rgb[1] = 128; point.rgb[2] =  67;
    PTG__AddPoint(ptg,&point);
    point.x[0] = 14.0; point.x[1] = 10.0; point.x[2] = 10.0; point.I    = 0.6; point.rgb[0] =   0; point.rgb[1] = 128; point.rgb[2] =  67;
    PTG__AddPoint(ptg,&point);

    point.x[0] = 21.0; point.x[1] = 10.0; point.x[2] = 10.0; point.I    = 0.6; point.rgb[0] =   0; point.rgb[1] = 128; point.rgb[2] =  67;
    PTG__AddPoint(ptg,&point);
    point.x[0] = 22.0; point.x[1] = 10.0; point.x[2] = 10.0; point.I    = 0.6; point.rgb[0] =   0; point.rgb[1] = 128; point.rgb[2] =  67;
    PTG__AddPoint(ptg,&point);
    PTG__AddPoint(ptg,NULL);
    PTG__AddPoint(ptg,NULL);

    point.x[0] = 31.0; point.x[1] = 10.0; point.x[2] = 10.0; point.I    = 0.6; point.rgb[0] =   0; point.rgb[1] = 128; point.rgb[2] =  67;
    PTG__AddPoint(ptg,&point);
    PTG__AddPoint(ptg,NULL);
    point.x[0] = 33.0; point.x[1] = 10.0; point.x[2] = 10.0; point.I    = 0.6; point.rgb[0] =   0; point.rgb[1] = 128; point.rgb[2] =  67;
    PTG__AddPoint(ptg,&point);
    PTG__AddPoint(ptg,NULL);

    PTG__Close(ptg);
  }
}

void test02(char *filename,char *outfilename)
{
  FILE *fpout;
  TPTG_point              point;
  TPTG_handle             ptg;
  TPTG_metadata_mandatory mdata;

  fpout = fopen(outfilename,"wt");

  ptg = PTG__Open(filename,&mdata);
  if (ptg!=-1)
  {
    int ic,ir;


    ProtoMeldung("start converting PTG-file '%s' to PTS-simular file '%s':\n",filename,outfilename);
    point.I = 0.0;
    point.rgb[0] = point.rgb[1] = point.rgb[2] = 0;
    for (ir=0;ir<mdata.rows;ir++ )
    {
      for (ic=0;ic<mdata.cols;ic++ )
      {
        if (PTG__GetPoint(ptg,&point))
        {
          //ProtoMeldung("%d %d : %f %f %f %f\n",ir,ic,point.x[0],point.x[1],point.x[2],point.I);
          fprintf(fpout,"%f %f %f %d %d %d %d\n",point.x[0],point.x[1],point.x[2],
                                           (int) (point.I*4096.0),
                                           point.rgb[0],point.rgb[1],point.rgb[2]);
        }
        else
        {
          // fprintf(fpout,"0 0 0  0.0  0 0 0\n");
        }
      }
	  Meldung("%4.0lf%%\b\b\b\b\b",((double)ir)/((double)mdata.rows)*100.0);
    }
	Meldung("\n");
    PTG__Close(ptg);
  }
  fclose(fpout);
}

void test03(char *filename)
{
  TPTG_point              point;
  TPTG_handle             ptg;
  TPTG_metadata_mandatory mdata;

  ptg = PTG__Open(filename,&mdata);
  if (ptg!=-1)
  {
    int ic,ir;
    int i,j,k;

    ProtoMeldung("random access to PTG-file [rows=%d cols=%d] '%s':\n",mdata.rows,mdata.cols,filename);
    point.I = 0.0;
    point.rgb[0] = point.rgb[1] = point.rgb[2] = 0;
    for (i=0;i<100;i++) 
		{
			k = 0;
		  clock_t clk,tim;

			clk = clock();
      for (j=0;j<10000-1;j++) 
	  	{
  		  ir = rand() * (mdata.rows-1) / (float) RAND_MAX;
	  	  ic = rand() * (mdata.cols-1) / (float) RAND_MAX;
        if (PTG__GetPointColRow(ptg,ic,ir,&point))
        {        
		  	  k++; 
			  }
			}
			tim = clock() - clk;
			j++;
  		ir = rand() * (mdata.rows-1) / (float) RAND_MAX;
	  	ic = rand() * (mdata.cols-1) / (float) RAND_MAX;
      Meldung("%8d [%6d,%6d]=",i*j,ir,ic);			
      if (PTG__GetPointColRow(ptg,ic,ir,&point))
      {        
			  Meldung("%8.2f %8.2f %8.2f %5d ",point.x[0],point.x[1],point.x[2],(int) (point.I*4096.0));
			}
			else Meldung("%17s %8s %5s ","not defined","","");
			Meldung("%6.4f s\n",tim/((float) CLK_TCK));
		}
  	Meldung("\n");
    PTG__Close(ptg);
  }
}

int main(int argc, char* argv[])
{
  FILE  *fperr;
  char  header[513];
  char  filename[512];
  char  outfilename[512];
  int   cnt;
  char  help[256];
  long int   total_numpts;
  long   fpos;

  fperr = fopen("PTG-DLL_test01.err","wt");

  ProtoMeldung("+-----------------------------------------------------------------+\n");
  sprintf(header,"| PTG-DLL_test01.exe %s %s |\n",PVERSION,PCOPYRIGHT);
  ProtoMeldung(header);
  ProtoMeldung("+-----------------------------------------------------------------+\n");
  PTG__About(header); 
  ProtoMeldung(header);

  if (argc > 2)
  {
    if (argv[1][0]=='-' || argv[2][0]=='-') Usage(argv[0]);
    else
    {
      strcpy(filename,argv[1]);
      strcpy(outfilename,argv[2]);
    }
  }
  else
  {
    Usage(argv[0]);
    fclose(fperr);
    exit(1);
  }


  ProtoMeldung("\nAufruf: ");
  for (int i=0;i<argc;i++) ProtoMeldung("%s ",argv[i]);
  ProtoMeldung("\n\n");

  ProtoMeldung("test01()\n");
  test01();

//  ProtoMeldung("test02()\n");
//  test02(filename,outfilename);

  ProtoMeldung("test03()\n");
  test03(filename);

  fclose(fperr);
  return 0;
}
