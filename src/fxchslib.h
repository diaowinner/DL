/*
   header of font file
*/

#ifndef FXCHSLIB_H
#define FXCHSLIB_H

typedef struct
{
	char			author[16]		;
	unsigned int	cwidth			;
	unsigned int	awidth,height	;
	unsigned int	asc_offset		;
	unsigned int	chs_offset		;
	int				file_handle		;
	int				cfont_size		;
	int				afont_size		;
}
FONTFILE;


extern const FONTFILE * sys_font,* usr_font;

FONTFILE *	open_font		(const char * cfname);
void		close_font		(const FONTFILE * ff);
void		select_font		(const FONTFILE * font);
void		print_chs_char	(int x,int y,int sel,unsigned char c1,unsigned char c2);
void		print_asc_char	(int x,int y,int sel,unsigned char c);
void		print_chs_str	(int x,int y,int sel,const unsigned char * str);
int			get_str_width	(const unsigned char * str);


#define Chs_OpenFont		open_font
#define Chs_CloseFont		close_font
#define Chs_SelectFont		select_font
#define Chs_Print			print_chs_str
#define Chs_PrintChsChar	print_chs_char
#define Chs_PrintAscChar	print_asc_char
#define Chs_GetStrWidth		get_str_width
#endif

