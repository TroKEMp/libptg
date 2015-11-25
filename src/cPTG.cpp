/*
  PTG
  ---

  Modul:    cPTG.h
  Version:  1.02
  Datum:    01.06.2010
  Autor:    Prof. Dr.-Ing. Fredie Kern
  Lizenz:   Open Source, GNU-Lizenz

  Beschreibung:

  Cyclone PTG File Format
  Deklarationsteil
  C-Source only

  v1.00  14.02.2010  Start der Implementierung
  v1.01  17.02.2010  erste veröffentlichte Version
  v1.02  01.06.2010  mit Transformation
	v1.03  10.06.2010  mit indiziertem Auslesen


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

#include "cPTG.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

void __default_echo(char *str,...)
{
}

// modul global variables

static T_cPTG_echo extern_error  =__default_echo;
static T_cPTG_echo extern_warning=__default_echo;

static char extern_creator[1024]="(default: " __FILE__ " " __DATE__ " " __TIME__ ")";
static int  DebugLevel=0;

static char **__list=NULL;
static int    __n=0;

static T_cPTG_metadata_mandatory *pMetaData=NULL;

static Tint64 *absolute_col_offsets;
static bool    iscreated=false;

// helper functions
void _cPTG_nogeoreference(double *p)
{
  for (int i=0;i<4;i++ )
  {
    for (int j=0;j<4;j++) p[i*4+j] = 0.0;
    p[i*4+i] = 1.0;
  }
}

void _cPTG_copygeoreference(double *p,double *q)
{
  for (int i=0;i<4;i++ )
  {
    for (int j=0;j<4;j++) p[i*4+j] = q[i*4+j];
  }
}

/*
char *_cPTG_new_string(char *str)
{
  __list      = (char **) realloc(__list,(__n+1)*sizeof(char *));
  __list[__n] = (char *)  calloc(strlen(str)+1,sizeof(char));
  strcpy(__list[__n],str);
  __n++;

  return(__list[__n-1]);
}
*/

// Level 0
void _cPTG_init(
                void (*warnings) (char *str,...),
                void (*errors)   (char *str,...),
                int debuglevel,
                char *creator
               )
{
  if (creator!=NULL) strcpy(extern_creator,creator);
  extern_error   = errors;
  extern_warning = warnings;
  DebugLevel     = debuglevel;
  iscreated = false;
}

void _cPTG_exit(void)
{
  for (int i=0;i<__n;i++ )
  {
    free(__list[i]);
  }
  free(__list);
  __list = NULL;
  __n = 0;
}

// Level 1  -- creation --

bool _cPTG_write_string(FILE *fp,char *str)
{
  Tint32 length;

  length = strlen(str)+1;

  fwrite(&length,sizeof(Tint32),1,fp);
  fwrite(str,sizeof(char),length,fp);
  return(true);
}

bool  _cPTG_write_metadata(T_cPTG_handle *handle)
{
  Tint32 magicnumber=MAGICNUMBER;

  fseek(handle->fp,0,SEEK_SET);

  fwrite(FILETYPETAG,sizeof(Tint8),4,handle->fp);
  fwrite(&magicnumber,sizeof(Tint32),1,handle->fp);

  _cPTG_write_string(handle->fp,"%%header_begin");
  _cPTG_write_string(handle->fp,"%%version");
  fwrite(&pMetaData->version,sizeof(Tint32),1,handle->fp);
  _cPTG_write_string(handle->fp,"%%cols");
  fwrite(&pMetaData->cols,sizeof(Tint32),1,handle->fp);
  _cPTG_write_string(handle->fp,"%%rows");
  fwrite(&pMetaData->rows,sizeof(Tint32),1,handle->fp);
  _cPTG_write_string(handle->fp,"%%properties");
  fwrite(&pMetaData->properties,sizeof(Tint32),1,handle->fp);
  _cPTG_write_string(handle->fp,"%%transform");
  fwrite(pMetaData->tmatrix,sizeof(Tdouble),4*4,handle->fp);
  _cPTG_write_string(handle->fp,"%%header_end");

  fwrite(absolute_col_offsets,sizeof(Tint64),pMetaData->cols,handle->fp);

  return(true);
}


T_cPTG_handle *_cPTG_create(char *fname,
                            T_cPTG_metadata_mandatory *mdata
                            )
{
  T_cPTG_handle *phandle;

  pMetaData = mdata;

  phandle = (T_cPTG_handle *) calloc(1,sizeof(T_cPTG_handle));

  if (phandle!=NULL)
  {
    phandle->fp = fopen(fname,"w+b");  // lesen+schreiben, eventuell vorhandene Datei wird überschrieben
    if (phandle->fp==NULL)
    {
      extern_error("can't open file '%s'",fname);
      free(phandle->fp);
      phandle->fp = NULL;
    }
    else
    {
      phandle->ok = true;
      iscreated = true;
      if (DebugLevel>0) extern_warning("file '%s' successfull opened",fname);

      absolute_col_offsets = (Tint64 *) calloc(sizeof(Tint64),mdata->cols);
      phandle->nrow     = ceil((double) mdata->rows/8.0);
      phandle->bitmask  = (Tint8  *)    calloc(sizeof(Tint8),phandle->nrow);

      switch(mdata->properties)
      {
        case DATATYPE_xyz:
          phandle->xyz_points     = (T_cPTG_xyz     *) calloc(sizeof(T_cPTG_xyz    ),mdata->rows);
          break;
        case DATATYPE_xyzI:
          phandle->xyzI_points    = (T_cPTG_xyzI    *) calloc(sizeof(T_cPTG_xyzI   ),mdata->rows);
          break;
        case DATATYPE_xyzrgb:
          phandle->xyzrgb_points  = (T_cPTG_xyzrgb  *) calloc(sizeof(T_cPTG_xyzrgb ),mdata->rows);
          break;
        case DATATYPE_xyzIrgb:
          phandle->xyzIrgb_points = (T_cPTG_xyzIrgb *) calloc(sizeof(T_cPTG_xyzIrgb),mdata->rows);
          break;
        default:
          break;
      }
      
      _cPTG_write_metadata(phandle);
      phandle->act_row = 0;
      phandle->act_col = 0;
      phandle->act_rowpos = 0;
    }
  }
  return(phandle);
}

bool _cPTG_write_row(T_cPTG_handle *handle)
{
  absolute_col_offsets[handle->act_col] = ftell(handle->fp);
  fwrite(handle->bitmask,sizeof(Tint8),handle->nrow,handle->fp);

  switch(pMetaData->properties)
  {
    case DATATYPE_xyz:
      fwrite(handle->xyz_points    ,sizeof(T_cPTG_xyz)   ,handle->act_rowpos,handle->fp);
      break;
    case DATATYPE_xyzI:
      fwrite(handle->xyzI_points   ,sizeof(T_cPTG_xyzI)  ,handle->act_rowpos,handle->fp);
      break;
    case DATATYPE_xyzrgb:
      fwrite(handle->xyzrgb_points ,sizeof(T_cPTG_xyzrgb),handle->act_rowpos,handle->fp);
      break;
    case DATATYPE_xyzIrgb:
      fwrite(handle->xyzIrgb_points,sizeof(T_cPTG_xyzIrgb),handle->act_rowpos,handle->fp);
      break;
    default:
      break;
  }
  return(true);
}

bool _cPTG_add_xyzIrgb(T_cPTG_handle *handle,T_cPTG_xyzIrgb *point)
{
  bool ok;

  ok = true;
  if (point!=NULL)
  {
    handle->bitmask[handle->act_row/8] = handle->bitmask[handle->act_row/8] | 1<<(7-handle->act_row%8);
    switch(pMetaData->properties)
    {
      case DATATYPE_xyz:
        for (int k=0;k<3;k++) handle->xyz_points[handle->act_rowpos].x[k] = point->x[k];
        break;
      case DATATYPE_xyzI:
        for (int k=0;k<3;k++) handle->xyzI_points[handle->act_rowpos].x[k] = point->x[k];
                              handle->xyzI_points[handle->act_rowpos].I    = point->I;
        break;
      case DATATYPE_xyzrgb:
        for (int k=0;k<3;k++) handle->xyzrgb_points[handle->act_rowpos].x[k]   = point->x[k];
        for (int k=0;k<3;k++) handle->xyzrgb_points[handle->act_rowpos].rgb[k] = point->rgb[k];
        break;
      case DATATYPE_xyzIrgb:
        for (int k=0;k<3;k++) handle->xyzIrgb_points[handle->act_rowpos].x[k]   = point->x[k];
                              handle->xyzIrgb_points[handle->act_rowpos].I      = point->I;
        for (int k=0;k<3;k++) handle->xyzIrgb_points[handle->act_rowpos].rgb[k] = point->rgb[k];
        break;
      default:
        break;
    }
    handle->act_rowpos++;
  }
  handle->act_row++;
  if (handle->act_row==pMetaData->rows)
  {
    _cPTG_write_row(handle);
    memset(handle->bitmask,0,sizeof(Tint8)*handle->nrow);
    handle->act_row = 0;
    handle->act_rowpos = 0;
    handle->act_col++;
  }
  return(ok);
}


// Level 1  -- opening --

bool _cPTG_read_string(FILE *fpin,char **str)
{
  Tint32 length;

  fread(&length,sizeof(Tint32),1,fpin);
  *str = (char *) realloc(*str,length*sizeof(char));
  fread(*str,sizeof(char),length,fpin);
  return(true);
}


bool _cPTG_read_metadata(T_cPTG_handle *handle,
                         T_cPTG_metadata_mandatory *mdata
                            )
{
  FILE      *fpin;
  FILE      *fpout;
  Tint8      filetype[10];
  Tint32     magicnumber;
  char      *key,*value;
  bool       isend;
  Tint32     dummy_int32;
  Tdouble    dummy_double;
  Tint32     properties;
  long int   total_numpts;
  bool       ret;

  key = value = NULL;
  ret = true;

  fpin = handle->fp;
  fread(filetype,sizeof(Tint8),4,fpin);
  if (strcmp(filetype,FILETYPETAG)==0)
  {
    fread(&magicnumber,sizeof(Tint32),1,fpin);
    if (magicnumber==MAGICNUMBER)
    {
      _cPTG_read_string(fpin,&key);
      if (strcmp(key,"%%header_begin")==0)
      {
        isend = false;
        do
        {
          _cPTG_read_string(fpin,&key);
          if (strcmp(key,"%%header_end")==0)
          {
            isend = true;
          }
          else if (strcmp(key,"%%version")==0)
          {
            fread(&(mdata->version),sizeof(Tint32),1,fpin);
            extern_warning("mandatory metadata '%s' found = %ld\n",key,mdata->version);
          }
          else if (strcmp(key,"%%cols")==0)
          {
            fread(&(mdata->cols),sizeof(Tint32),1,fpin);
            extern_warning("mandatory metadata '%s' found = %ld\n",key,mdata->cols);
          }
          else if (strcmp(key,"%%rows")==0)
          {
            fread(&(mdata->rows),sizeof(Tint32),1,fpin);
            extern_warning("mandatory metadata '%s' found = %ld\n",key,mdata->rows);
          }
          else if (strcmp(key,"%%transform")==0)
          {
            fread(mdata->tmatrix,sizeof(Tdouble),4*4,fpin);
            extern_warning("mandatory metadata '%s' found = \n",key);
            for (int i=0;i<4;i++)
            {
              extern_warning("  ");
              for (int j=0;j<4;j++)
              {
                 extern_warning("%12.6lf ",mdata->tmatrix[i*4+j]);
              }
              extern_warning("\n");
            }
          }
          else if (strcmp(key,"%%properties")==0)
          {
            fread(&(mdata->properties),sizeof(Tint32),1,fpin);
            extern_warning("mandatory metadata '%s' found = %04X\n",key,mdata->properties);
            extern_warning("  property xyz as float  = '%s'\n",mdata->properties & PROP_XYZFLOAT  ? "yes" : "no");
            extern_warning("  property xyz as double = '%s'\n",mdata->properties & PROP_XYZDOUBLE ? "yes" : "no");
            extern_warning("  property intensity     = '%s'\n",mdata->properties & PROP_INTENSITY ? "yes" : "no");
            extern_warning("  property RGB           = '%s'\n",mdata->properties & PROP_RGBVALUE  ? "yes" : "no");
          }
          else if (strcmp(key,"%%rows_total")==0)
          {
            fread(&dummy_int32,sizeof(Tint32),1,fpin);
            extern_warning("optional metadata '%s' found = %d\n",dummy_int32);
          }
          else if (strcmp(key,"%%azim_max")==0)
          {
            fread(&dummy_double,sizeof(Tdouble),1,fpin);
            extern_warning("optional metadata '%s' found = %lf\n",dummy_double);
          }
          else if (strcmp(key,"%%azim_min")==0)
          {
            fread(&dummy_double,sizeof(Tdouble),1,fpin);
            extern_warning("optional metadata '%s' found = %lf\n",dummy_double);
          }
          else if (strcmp(key,"%%elev_max")==0)
          {
            fread(&dummy_double,sizeof(Tdouble),1,fpin);
            extern_warning("optional metadata '%s' found = %lf\n",dummy_double);
          }
          else if (strcmp(key,"%%elev_min")==0)
          {
            fread(&dummy_double,sizeof(Tdouble),1,fpin);
            extern_warning("optional metadata '%s' found = %lf\n",dummy_double);
          }
          else // strings
          {
            _cPTG_read_string(fpin,&value);
            extern_warning("optional metadata '%s' found value is '%s'\n",key,value);
          }
        }
        while(!isend);
      }
      free(key);
      free(value);
    }
    else
    {
      extern_error("ERROR: wrong magic number. '%x'expected but '%x' is read !",magicnumber,MAGICNUMBER);
      ret = false;
    }
  }
  else
  {
     extern_error("ERROR: file type tag 'PTG' expected and not '%s'!",filetype);
     ret = false;
  }

  return(ret);
}

bool _cPTG_read_row(T_cPTG_handle *handle)
{
  fseek(handle->fp,absolute_col_offsets[handle->act_col],SEEK_SET);
  fread(handle->bitmask,sizeof(Tint8),handle->nrow,handle->fp);

  handle->numpts = 0;
  for (int j=0;j<handle->nrow;j++)
  {
     if (handle->bitmask[j] & 0x80) {
       //ProtoMeldung("1");
       handle->numpts++;
     } //else ProtoMeldung("0");
     if (handle->bitmask[j] & 0x40) {
       //ProtoMeldung("1");
       handle->numpts++;
     } //else ProtoMeldung("0");
     if (handle->bitmask[j] & 0x20) {
       //ProtoMeldung("1");
       handle->numpts++;
     } //else ProtoMeldung("0");
     if (handle->bitmask[j] & 0x10) {
       //ProtoMeldung("1");
       handle->numpts++;
     } //else ProtoMeldung("0");
     if (handle->bitmask[j] & 0x08) {
       //ProtoMeldung("1");
       handle->numpts++;
     } //else ProtoMeldung("0");
     if (handle->bitmask[j] & 0x04) {
       //ProtoMeldung("1");
       handle->numpts++;
     } //else ProtoMeldung("0");
     if (handle->bitmask[j] & 0x02) {
       //ProtoMeldung("1");
       handle->numpts++;
     } //else ProtoMeldung("0");
     if (handle->bitmask[j] & 0x01) {
       //ProtoMeldung("1");
       handle->numpts++;
     } //else ProtoMeldung("0");
  }
  switch(pMetaData->properties)
  {
    case DATATYPE_xyz:
      fread(handle->xyz_points    ,sizeof(T_cPTG_xyz)   ,handle->numpts,handle->fp);
      break;
    case DATATYPE_xyzI:
      fread(handle->xyzI_points   ,sizeof(T_cPTG_xyzI)  ,handle->numpts,handle->fp);
      break;
    case DATATYPE_xyzrgb:
      fread(handle->xyzrgb_points ,sizeof(T_cPTG_xyzrgb),handle->numpts,handle->fp);
      break;
    case DATATYPE_xyzIrgb:
      fread(handle->xyzIrgb_points,sizeof(T_cPTG_xyzIrgb),handle->numpts,handle->fp);
      break;
    default:
      break;
  }
  return(true);
}

T_cPTG_handle *_cPTG_open(char *fname,
                          T_cPTG_metadata_mandatory *mdata
                          )
{
  T_cPTG_handle *phandle;

  pMetaData = mdata;
  phandle = (T_cPTG_handle *) calloc(1,sizeof(T_cPTG_handle));

  if (phandle!=NULL)
  {
    phandle->fp = fopen(fname,"r+b");  // lesen+schreiben, vorhandene Datei
    if (phandle->fp==NULL)
    {
      extern_error("can't open file '%s'",fname);
      free(phandle->fp);
      phandle->fp = NULL;
      _cPTG_close(phandle);
      phandle = NULL;
    }
    else
    {
      T_cPTG_echo m_echo;

      phandle->ok = true;
      iscreated   = false;
      if (DebugLevel>0) extern_warning("file '%s' successfull opened",fname);

      m_echo         = extern_warning;
      extern_warning = __default_echo;
      _cPTG_read_metadata(phandle,pMetaData);
      extern_warning = m_echo;

/* data */
      absolute_col_offsets = (Tint64 *) calloc(sizeof(Tint64),mdata->cols);
      phandle->nrow     = ceil((double) mdata->rows/8.0);
      phandle->bitmask  = (Tint8  *)    calloc(sizeof(Tint8),phandle->nrow);

      fread(absolute_col_offsets,sizeof(Tint64),mdata->cols,phandle->fp);
      switch(mdata->properties)
      {
        case DATATYPE_xyz:
          phandle->xyz_points     = (T_cPTG_xyz     *) calloc(sizeof(T_cPTG_xyz    ),mdata->rows);
          break;
        case DATATYPE_xyzI:
          phandle->xyzI_points    = (T_cPTG_xyzI    *) calloc(sizeof(T_cPTG_xyzI   ),mdata->rows);
          break;
        case DATATYPE_xyzrgb:
          phandle->xyzrgb_points  = (T_cPTG_xyzrgb  *) calloc(sizeof(T_cPTG_xyzrgb ),mdata->rows);
          break;
        case DATATYPE_xyzIrgb:
          phandle->xyzIrgb_points = (T_cPTG_xyzIrgb *) calloc(sizeof(T_cPTG_xyzIrgb),mdata->rows);
          break;
        default:
          break;
      }

      phandle->act_row = 0;
      phandle->act_col = 0;
      phandle->act_rowpos = 0;
      _cPTG_read_row(phandle);
    }
  }
  return(phandle);
}

bool _cPTG_get_xyzIrgb(T_cPTG_handle *handle,T_cPTG_xyzIrgb *point)
{
  bool ok;

  ok = handle->bitmask[handle->act_row/8] & 1<<(7-handle->act_row%8);
  if (ok)
  {
    switch(pMetaData->properties)
    {
      case DATATYPE_xyz:
        for (int k=0;k<3;k++) point->x[k] = handle->xyz_points[handle->act_rowpos].x[k];
        break;
      case DATATYPE_xyzI:
        for (int k=0;k<3;k++) point->x[k] = handle->xyzI_points[handle->act_rowpos].x[k];
                              point->I    = handle->xyzI_points[handle->act_rowpos].I;
        break;
      case DATATYPE_xyzrgb:
        for (int k=0;k<3;k++) point->x[k]   = handle->xyzrgb_points[handle->act_rowpos].x[k];
        for (int k=0;k<3;k++) point->rgb[k] = handle->xyzrgb_points[handle->act_rowpos].rgb[k];
        break;
      case DATATYPE_xyzIrgb:
        for (int k=0;k<3;k++) point->x[k]   = handle->xyzIrgb_points[handle->act_rowpos].x[k];
                              point->I      = handle->xyzIrgb_points[handle->act_rowpos].I;
        for (int k=0;k<3;k++) point->rgb[k] = handle->xyzIrgb_points[handle->act_rowpos].rgb[k];
        break;
      default:
        break;
    }
    handle->act_rowpos++;
  }
  handle->act_row++;
  if (handle->act_row==pMetaData->rows)
  {
    handle->act_row = 0;
    handle->act_col++;
    if (handle->act_col<pMetaData->cols)
    {
      _cPTG_read_row(handle);
      handle->act_rowpos = 0;
    }
    else
    {
      ok = false; // Fehler
      if (handle->act_col>pMetaData->cols)
        extern_warning("can't read more than rows times cols points!!!");
    }
  }
  return(ok);
}

bool _cPTG_get_xyzIrgb(T_cPTG_handle *handle,int col, int row, T_cPTG_xyzIrgb *point)
{
	bool ok;

	ok = true;
	if (handle->act_col!=col)
	{
		handle->act_col = col;
    ok = _cPTG_read_row(handle);
	}
	if (ok)
	{
		int cnt,i,j,bit;

    handle->act_row = row;
    ok = handle->bitmask[handle->act_row/8] & 1<<(7-handle->act_row%8);
    if (ok)
    {
			handle->act_rowpos = 0;
			cnt = 0;
			j = 0;
			while (cnt<handle->act_row)
			{ 
			  bit = 0x80;
			  i   = 0;
			  while (i<8 && cnt<row)
			  {
          if (handle->bitmask[j] & bit) {
             handle->act_rowpos++;
          } 
				  cnt++;
				  bit = bit / 2;
					i++;
			  }
				j++;
			}
      switch(pMetaData->properties)
      {
        case DATATYPE_xyz:
          for (int k=0;k<3;k++) point->x[k] = handle->xyz_points[handle->act_rowpos].x[k];
          break;
        case DATATYPE_xyzI:
          for (int k=0;k<3;k++) point->x[k] = handle->xyzI_points[handle->act_rowpos].x[k];
                                point->I    = handle->xyzI_points[handle->act_rowpos].I;
          break;
        case DATATYPE_xyzrgb:
          for (int k=0;k<3;k++) point->x[k]   = handle->xyzrgb_points[handle->act_rowpos].x[k];
          for (int k=0;k<3;k++) point->rgb[k] = handle->xyzrgb_points[handle->act_rowpos].rgb[k];
          break;
        case DATATYPE_xyzIrgb:
          for (int k=0;k<3;k++) point->x[k]   = handle->xyzIrgb_points[handle->act_rowpos].x[k];
                                point->I      = handle->xyzIrgb_points[handle->act_rowpos].I;
          for (int k=0;k<3;k++) point->rgb[k] = handle->xyzIrgb_points[handle->act_rowpos].rgb[k];
          break;
        default:
          break;
      }
    }
//		ok = _cPTG_get_xyzIrgb(handle,point);
	}
	return(ok);
}

// Level 1  -- closeing --

void _cPTG_close(T_cPTG_handle *handle)
{
  if (handle!=NULL && handle->ok)
  {
    if (iscreated) _cPTG_write_metadata(handle);
    fclose(handle->fp);
    if (DebugLevel>0) extern_warning("handle successfull closed");

    free(absolute_col_offsets);
    free(handle->bitmask);
    switch(pMetaData->properties)
    {
      case DATATYPE_xyz:
        free(handle->xyz_points);
        break;
      case DATATYPE_xyzI:
        free(handle->xyzI_points);
        break;
      case DATATYPE_xyzrgb:
        free(handle->xyzrgb_points);
        break;
      case DATATYPE_xyzIrgb:
        free(handle->xyzIrgb_points);
        break;
      default:
        break;
    }
    free(handle);
    handle = NULL;
    pMetaData = NULL;
  }
  else
  {
    extern_error("unknown handle");
  }
}

void _cPTG_Local2World(T_cPTG_handle *handle,T_cPTG_xyzIrgb *point,double *X)
{

  X[0] = point->x[0] * pMetaData->tmatrix[0*4+0] +
         point->x[1] * pMetaData->tmatrix[1*4+0] +
         point->x[2] * pMetaData->tmatrix[2*4+0] +
                       pMetaData->tmatrix[3*4+0];
  X[1] = point->x[0] * pMetaData->tmatrix[0*4+1] +
         point->x[1] * pMetaData->tmatrix[1*4+1] +
         point->x[2] * pMetaData->tmatrix[2*4+1] +
                       pMetaData->tmatrix[3*4+1];
  X[2] = point->x[0] * pMetaData->tmatrix[0*4+2] +
         point->x[1] * pMetaData->tmatrix[1*4+2] +
         point->x[2] * pMetaData->tmatrix[2*4+2] +
                       pMetaData->tmatrix[3*4+2];
}

