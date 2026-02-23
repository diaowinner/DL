/******************************************************************
* DLiterature  v2.1.3  大小写修复版           *
*******************************************************************/

#include "dl2_all.h"

#define SCR_WIDTH		128
#define SCR_HEIGHT		64

#define MARK_SIZE		5

#define PAGE_MAX		(9999)
#define BUFFER_SIZE		(20*1024)
int     font_modle = 12;
word		*	page_offset			;
uint			page_size	=	0	;
uint			page_now	=	0	;
uint			file_size	=	0	;
uint			file_pos	=	0	;
const char	*	file_name			;
int				fh_txt		=	0	;
long			mark[MARK_SIZE]		;


const char * MAIN_MENU_TEXT[] = {" 打开闪存文件 "," 打开SD卡文件 "," 更改阅读字体 "," 关于 "};
const char * READ_MENU_TEXT[] = { " 更改字体 "," 跳页 "," 读取书签 "," 保存书签 "," 退出 "};

void save_page_mark ()
{
	char	mark_file_name[32];
	int		file;
	
	strcpy(mark_file_name,file_name);
	memcpy(strstr(mark_file_name,".txt"),".svt",4);

	file = open_file(mark_file_name,_OPENMODE_WRITE);
	
    if(file >= 0) {
	    write_file(file,&page_now,sizeof(int));
	    write_file(file,mark,sizeof(int)*MARK_SIZE);
	    close_file(file);
    }
}


void read_page_mark ()
{
	char	mark_file_name[32];
	int		file;
	
	strcpy(mark_file_name,file_name);
	memcpy(strstr(mark_file_name,".txt"),".svt",4);

	file = open_file(mark_file_name,_OPENMODE_READ);
	
	if (file<0)
	{
		int 	i;
		fontc 	ffname[32];
		page_now = 0;
		for(i=0;i<MARK_SIZE;++i)
		{
			mark[i] = 0;
		}
		char_to_font(mark_file_name,ffname);
		i = create_file(ffname,sizeof(int)*(MARK_SIZE+1));
	}
	else
	{
		read_file(file,&page_now,sizeof(int),0);
		read_file(file,mark,sizeof(int)*MARK_SIZE,sizeof(int));
		close_file(file);
	}
}

int split_page ()
{
	uchar		*p;
	uchar		*buf;
	int			bytes_read,page_bytes;
	int			x,y;
	BOOL		last_page;

	page_size		= 0;
	file_pos		= 0;
	file_size		= get_file_size(fh_txt);
	buf				= (uchar*)calloc(BUFFER_SIZE-1,1);
	if (buf==NULL) return -2;

	select_font (usr_font);
	
	while(file_pos < file_size-1)
	{
		if (file_size - file_pos > BUFFER_SIZE)
			bytes_read = read_file(fh_txt,buf,BUFFER_SIZE,file_pos);
		else
			bytes_read = read_file(fh_txt,buf,file_size - file_pos,file_pos);

		buf[bytes_read] = '\0';
		
		p = buf;
		x = 0,y = 0;
		
		bytes_read = 0;
		page_bytes = 0;

		while(1)
		{
			if ((*p) & 0x80)
			{
				p+=2;
				page_bytes+=2;
				x += usr_font->cwidth;
			}
			else if ((*p)=='\n')
			{
				x = SCR_WIDTH;
				p++;
				page_bytes++;
			}
			else if (*p == 0)
			{
				break;
			}
			else
			{
				++p;
				page_bytes++;
			}
			if (x+usr_font->cwidth > SCR_WIDTH)
			{
				x = 0;
				y += usr_font->height;
				if (y + usr_font->height > SCR_HEIGHT)
				{
					y = 0;
					bytes_read += page_bytes;
					if (page_size>=PAGE_MAX)
					{
						free(buf);	
						return -1;
					}
					page_offset[page_size++] = page_bytes;
					page_bytes = 0;
					if (*p==0) break;
				}
			}
		}
		if (bytes_read==0) break;
		else
			file_pos += bytes_read;
	}
	free(buf);
	return 0;
}

long get_page_offset ()
{
	int offset = 0,i;
	for (i=0;i<page_now;++i)
	{
		offset+=page_offset[i];	
	}
	return offset;
}

int select_page_mark (const char * title)
{
	int				left = 64-get_str_width(title)/2,index = 0;
	BOOL 			redraw = TRUE;
	char			buf[128];
	unsigned int 	key;

	select_font(sys_font);
	all_clr();
	print_chs_str(left,0,0,title);
	while(1)
	{
		if (redraw)
		{
			int i,y=sys_font->height+4,x=0;
            all_clr();
	        print_chs_str(left,0,0,title);
			for (i=0;i<MARK_SIZE;++i,y+=sys_font->height)
			{
				sprintf(buf," -%d- %04d/%04d  ",i,mark[i]+1,page_size);
				print_chs_str(x,y,index==i,buf);
			}
			redraw = FALSE;
		}
		GetKey(&key);
		if (key==KEY_CTRL_UP)
		{
			if (index<=0) continue;
			--index;
			redraw = TRUE;
		}
		else if (key==KEY_CTRL_DOWN)
		{
			if (index>=MARK_SIZE-1) continue;
			++index;
			redraw = TRUE;
		}
		else if (key==KEY_CTRL_EXE)
		{
			return index;
		}
		else if (key==KEY_CTRL_EXIT) return -1;
	}
}

void start_read ()
{
	uint	key;
	int		redraw = 1;
	int		menu_index = 0;
    FONTFILE * temp_loader = NULL;

	file_pos = get_page_offset();
	
	while(1)
	{
		if (redraw)
		{
			uchar c1,c2;
			int   x = 0,y = 0,i = 0;
			redraw = 0;
			all_clr();
			select_font(usr_font);

			while(file_pos+i<file_size)
			{
				read_file(fh_txt,&c1,1,file_pos+i);i++;
				if (c1 & 0x80)
				{
					read_file(fh_txt,&c2,1,file_pos+i);i++;
					print_chs_char(x,y,0,c1,c2);
					x += usr_font->cwidth;
				}
				else if (c1 == '\n')
				{
					x = SCR_WIDTH;
				}
				else
				{
					print_asc_char(x,y,0,c1);
					x += usr_font->awidth;
				}
				if (x+usr_font->cwidth > SCR_WIDTH)
				{
					x = 0;
					y += usr_font->height;
					if (y + usr_font->height > SCR_HEIGHT)
						break;
				}
			}
		}
		GetKey(&key);
		if (key==KEY_CTRL_UP)
		{
			if (page_now<=0) continue;
			file_pos -= page_offset[--page_now];
			redraw = 1;
		}
		if (key==KEY_CTRL_DOWN)
		{
			if (page_now+1>=page_size) continue;
			file_pos += page_offset[page_now++];
			redraw = 1;
		}
		else if (key==KEY_CTRL_EXIT)
		{
			char	buf[128];
			int		r;
			select_font (sys_font);
			sprintf(buf,"  当前页%04d/%04d",page_now+1,page_size);
			all_clr();
			print_chs_str(0,0,0,buf);
			menu_index = show_menu (0,5,READ_MENU_TEXT,32,16);
			if (menu_index==-1) continue;
			else if (menu_index == 0)
			{
				/* 【修复重点：阅读内切换逻辑】 */
				if (font_modle == 12) {
                    // 想切小字体，既然菜单显示正常，直接借用 sys_font 绝对不出错
                    usr_font = (FONTFILE*)sys_font;
                    font_modle = 8;
                    show_dialog(" 字体 ", "已更换为小字体.", 0, DLG_BTN_OK);
                } else {
                    // 尝试切回 12 号，自动兼容大小写
                    temp_loader = open_font("\\\\fls0\\FONT12.dlf");
                    if (temp_loader == NULL) temp_loader = open_font("\\\\fls0\\font12.dlf");

                    if (temp_loader != NULL) {
                        usr_font = temp_loader;
                        font_modle = 12;
                        show_dialog(" 字体 ", "已更换为中字体.", 0, DLG_BTN_OK);
                    } else {
                        // 实在不行就继续用小字体顶着，决不能变白
                        usr_font = (FONTFILE*)sys_font;
                        show_dialog(" 提示 ", "无法加载，维持原样.", DLG_ICON_NO, DLG_BTN_OK);
                    }
                }
			}
			else if (menu_index==1)
			{
				show_dialog (""," 输入页码 ",0,0);
				sprintf(buf,"%d",page_now+1);
				if (get_line_box(buf,128,16,14,32))
				{
					page_now = atol(buf)-1;
					if (page_now<0) page_now=0;
					else if (page_now>=page_size)page_now = page_size-1;
					file_pos = get_page_offset();
				}
			}
			else if (menu_index==2)
			{
				r = select_page_mark ("读取书签");
				if (r!=-1)
				{
					page_now = mark[r];
					file_pos = get_page_offset();
				}
			}
			else if (menu_index==3)
			{
				r = select_page_mark ("保存书签");
				if (r!=-1)
				{
					mark[r] = page_now;
				}
			}
			else if (menu_index==4)
			{
				break;
			}
			redraw = TRUE;
		}//
	}
}

void start_read_text (const char * t_file_name)
{
	file_name = t_file_name;

	fh_txt = open_file(file_name,_OPENMODE_READ_SHARE);

	if (fh_txt<0)
	{
		show_dialog(" 错误 "," 找不到文件 ",DLG_ICON_WARN,DLG_BTN_OK);
		return;
	}

	show_dialog(" 分页中 "," 请稍等 ",DLG_ICON_WAIT,0);putdisp();
	switch(split_page ())
	{
		case -1:show_dialog(" 错误 "," 页数过多 ",DLG_ICON_WARN,DLG_BTN_OK);return;
		case -2:show_dialog(" 错误 "," 内存不足 ",DLG_ICON_WARN,DLG_BTN_OK);return;
		default:;
	}
	read_page_mark();
	start_read();
	save_page_mark();
	close_file(fh_txt);
}

int show_menu (int def_index,int count,const char * item[],int x,int y)
{
	int		width = 0,height,i,t,index = def_index;
	BOOL	redraw = TRUE;
	unsigned int key;
	
	select_font(sys_font);
	
	for (i=0;i<count;++i)
	{
		t = get_str_width(item[i]);
		if (t>width) width = t;
	}
	width;
	height = count * sys_font->height ;
	area_clear(x,y,x+width+1,y+height+1,2);
	while(1)
	{
		if (redraw)
		{
			int cx = x+1,cy = y+1;
			area_clear(x,y,x+width+1,y+height+1,2);
			for (i=0;i<count;++i)
			{
				print_chs_str(cx,cy,FALSE,item[i]);
				if (index==i) area_rev(cx,cy,cx+width-1,cy+sys_font->height-1);
				cy += sys_font->height;
			}
			redraw = FALSE;
		}
		GetKey(&key);
		if (key==KEY_CTRL_UP)
		{
			index--;if (index<0)index = count-1;
			redraw = TRUE;
		}
		else if (key==KEY_CTRL_DOWN)
		{
			index++;if (index>=count)index = 0;
			redraw = TRUE;
		}
		else if (key==KEY_CTRL_EXE)
		{
			return index;
		}
		else if (key==KEY_CTRL_EXIT)
		{
			return -1;
		}
	}
	return -1;
}

int AddIn_main(int isAppli, unsigned short OptionNum)
{
	int menu_index = 0,r;
    char temp_file_name[128];
    FONTFILE * temp_loader = NULL;

	/* 【初始化：菜单设为 8，阅读设为 12】 */
    // 菜单强制用 8 号字，自动兼容大小写
	sys_font = open_font("\\\\fls0\\FONT8.dlf");
	if (sys_font == NULL) sys_font = open_font("\\\\fls0\\font8.dlf");

    // 阅读默认用 12 号字
	usr_font = open_font("\\\\fls0\\FONT12.dlf");
    if (usr_font == NULL) usr_font = open_font("\\\\fls0\\font12.dlf");

    // 如果阅读用的 12 号字加载失败了，强行把阅读器指向菜单的 8 号字内存指针。
    // 这样绘图函数检查 if(def_font==NULL) 就绝对不会触发，文字必出！
	if (usr_font == NULL) usr_font = (FONTFILE*)sys_font;
    if (sys_font == NULL) sys_font = (FONTFILE*)usr_font;

	if (sys_font == NULL)
	{
		printmini(0,0,(uchar*)"ERROR: All fonts missing!",MINI_OVER);
		waitkey();
		return 0;
	}
	
	page_offset = (word*)calloc(PAGE_MAX,sizeof(word));
	if (page_offset==NULL)
	{
		show_dialog(" 错误 "," 内存不足 ",DLG_ICON_WARN,DLG_BTN_OK);
		return 0;
	}

	while (1)
	{
		all_clr();
		PrintXY(0,0,"  DLiterature 2.1.3",0);
		menu_index = show_menu (menu_index,4,MAIN_MENU_TEXT,24,16);
		if (menu_index==-1) break;
		else if (menu_index==0)
		{
			r = file_view(1,"txt",temp_file_name);
			if (r==1)			start_read_text(temp_file_name);
			else if (r==-1)		show_dialog(" 错误 "," 闪存中无此类文件 ",DLG_ICON_WARN,DLG_BTN_OK);
		}
		else if (menu_index==1)
		{
			r = file_view(0,"txt",temp_file_name);
			if (r==1)			start_read_text(temp_file_name);
			else if (r==-1)		show_dialog(" 错误 "," SD卡中无此类文件 ",DLG_ICON_WARN,DLG_BTN_OK);
		}
		else if (menu_index==2)
		{
            /* 【主循环切换：逻辑同理】 */
			if (font_modle == 12) {
                // 切 8 号，既然菜单在用，直接借用，决不读硬盘
				usr_font = (FONTFILE*)sys_font; 
				show_dialog(" 字体 ", "已更换为小字体.", 0, DLG_BTN_OK);
				font_modle = 8;
			} else {
                // 切 12 号，自动兼容大小写
				temp_loader = open_font("\\\\fls0\\FONT12.dlf");
                if (temp_loader == NULL) temp_loader = open_font("\\\\fls0\\font12.dlf");

                if (temp_loader != NULL) {
                    usr_font = temp_loader;
                    font_modle = 12;
                    show_dialog(" 字体 ", "已更换为中字体.", 0, DLG_BTN_OK);
                } else {
                    usr_font = (FONTFILE*)sys_font;
                }
			}
		}
		else if (menu_index==3)
		{
			show_dialog(" 关于 ","DL v2.1.3 Stable Fix. github.com/diaowinner/DL",0,DLG_BTN_OK);
		}
	}

	free(page_offset);
    // 这里也要做保护，不要关闭同一个指针两次
	if (usr_font != sys_font) close_font(usr_font);
	close_font(sys_font);

	return 0;
}

#pragma section _BR_Size
unsigned long BR_Size;
#pragma section
#pragma section _TOP
int InitializeSystem(int isAppli, unsigned short OptionNum)
{
    return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}
#pragma section