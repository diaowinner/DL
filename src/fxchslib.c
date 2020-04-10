/*
    Chinese Display Library

        by Anderain Lovelace 2012.8.19
*/

#include <stdio.h>
#include <fxlib.h>

#include "fxchslib.h"

#define is_chschar(c) ((c) & 0x80)


typedef FONTCHARACTER fontc;

const FONTFILE * def_font = NULL;
const FONTFILE * sys_font = NULL, * usr_font = NULL;

static void draw_picture (int x,int y,int width,int height,int mode,unsigned char * pGraph)
{
	DISPGRAPH dg;

	dg.x					= x;
	dg.y					= y;
	dg.GraphData.width		= width;
	dg.GraphData.height		= height;
	dg.GraphData.pBitmap	= pGraph;
	dg.WriteModify			= mode+1;
	dg.WriteKind			= IMB_WRITEKIND_OVER;

	Bdisp_WriteGraph_VRAM(&dg);
}

static unsigned int x86_dword_to_sh (unsigned int d)
{
	unsigned char c1,c2,c3,c4;
	unsigned int  r;

	c1 = d>>24	;
	c2 = (d & 0xFF0000)		>>16;
	c3 = (d & 0xFF00)		>>8	;
	c4 = (d & 0xFF)				;
	
	r = c1 + (c2<<8) + (c3<<16) + (c4<<24);

	return r;
}

static fontc * char_to_font(const char *cfname,fontc *ffname)
{
	int i,len = strlen(cfname);

	for(i=0; i<len ;++i)
		ffname[i] = cfname[i];

	ffname[i]=0;
	return ffname;
}

FONTFILE * open_font(const char * cfname)
{
	int			fh,r;
	FONTFILE	*ff;
	fontc		ffname[32];

	char_to_font(cfname,ffname);

	fh = Bfile_OpenFile(ffname,_OPENMODE_READ_SHARE);

	if (fh<=0) return NULL;

	ff			 		= (FONTFILE*)malloc(sizeof(FONTFILE));
	Bfile_ReadFile (fh,ff,sizeof(FONTFILE)-2*sizeof(int),0);

	ff->file_handle		= fh;

	ff->awidth			= x86_dword_to_sh(ff->awidth);
	ff->cwidth			= x86_dword_to_sh(ff->cwidth);
	ff->height			= x86_dword_to_sh(ff->height);	
	ff->asc_offset		= x86_dword_to_sh(ff->asc_offset);
	ff->chs_offset		= x86_dword_to_sh(ff->chs_offset);

	r					= ff->awidth/8 + (ff->awidth % 8 ? 1 : 0);
	ff->afont_size		= r*ff->height;
	r					= ff->cwidth/8 + (ff->cwidth % 8 ? 1 : 0);
	ff->cfont_size		= r*ff->height;
	return ff;
}

void close_font (const FONTFILE * ff)
{
	Bfile_CloseFile(ff->file_handle);
	free(ff);
}


void select_font (const FONTFILE * font)
{
	def_font = font;
}

void print_chs_char (int x,int y,int sel,unsigned char c1,unsigned char c2)
{
	unsigned char mat[128];
	int sec,pot;

	if (def_font==NULL) return;

	sec = c1-0xa1;
	pot = c2-0xa1;

	Bfile_ReadFile( def_font->file_handle,
					mat,
					def_font->cfont_size,
					(94*sec+pot)*def_font->cfont_size + def_font->chs_offset);

	draw_picture(x,y,def_font->cwidth,def_font->height,sel,mat);
}

void print_asc_char (int x,int y,int sel,unsigned char c)
{
	unsigned char mat[128];

	if (def_font==NULL) return;


	Bfile_ReadFile( def_font->file_handle,
					mat,
					def_font->afont_size,
					c*def_font->afont_size + def_font->asc_offset);

	draw_picture(x,y,def_font->awidth,def_font->height,sel,mat);
}

void print_chs_str (int x,int y,int sel,const unsigned char * str)
{
	int		i;
	int		l = 0;
	int		cx = x;

	if (! def_font) return ;

	for (i=0;str[i];l++)
	{
		if (is_chschar(str[i]))
		{
			print_chs_char (cx,y,sel,str[i],str[i+1]);
			++i,++i;
			cx += def_font->cwidth;
		}
		else
		{
			++i;
			if (! def_font->asc_offset) continue;
			print_asc_char (cx,y,sel,str[i-1]);
			cx += def_font->awidth;
		}		
	}
}

int get_str_width (const unsigned char * str)
{
	int width = 0;
	while(*str)
	{
		if (is_chschar(*str))
		{
			str++;str++;width+=def_font->cwidth;
		}
		else
		{
			str++;width+=def_font->awidth;
		}
	}
	return width;
}

