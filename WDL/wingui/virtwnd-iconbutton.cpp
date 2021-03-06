/*
    WDL - virtwnd-iconbutton.cpp
    Copyright (C) 2006 and later Cockos Incorporated

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.
      

    Implementation for virtual window icon buttons, icons, combo boxes, and static text controls.

*/

#include "virtwnd-controls.h"
#include "../lice/lice.h"

WDL_VirtualIconButton::WDL_VirtualIconButton()
{
  m_alpha=1.0;
  m_checkstate=-1;
  m_textfont=0;
  m_textalign=0;
  m_bgcol1_msg=0;
  m_is_button=true;
  m_pressed=0;
  m_iconCfg=0;
  m_en=true;
  m_grayed = false;
  m_forceborder=false;
  m_forcetext=false;
  m_ownsicon=false;
  m_immediate=false;
  m_margin_r = m_margin_l = 0;
  m_margin_t = m_margin_b = 0;
}

WDL_VirtualIconButton::~WDL_VirtualIconButton()
{
  if (m_ownsicon && m_iconCfg)
  {
    delete m_iconCfg->image;
    delete m_iconCfg;
  }
}

void WDL_VirtualIconButton::SetTextLabel(const char *text, char align, LICE_IFont *font) 
{ 
  if (font) m_textfont=font;
  m_textalign=align;
  m_textlbl.Set(text); 
  if (!m_iconCfg || m_forcetext) RequestRedraw(NULL); 
} 

void WDL_VirtualIconButton::SetCheckState(char state)
{
  if (state != m_checkstate)
  {
    m_checkstate=state;
    RequestRedraw(NULL);
  }
}

void WDL_VirtualIconButton::SetIcon(WDL_VirtualIconButton_SkinConfig *cfg, float alpha, bool buttonownsicon)
{ 
  if (m_iconCfg != cfg || m_alpha != alpha) 
  {
    if (m_ownsicon && m_iconCfg && m_iconCfg != cfg)
    {
      delete m_iconCfg->image;
      delete m_iconCfg;
    }
    m_alpha=alpha; 
    m_iconCfg=cfg;
    RequestRedraw(NULL); 
  }
  m_ownsicon = buttonownsicon;
 }

void WDL_VirtualIconButton::OnPaintOver(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect)
{
  if (m_iconCfg && m_iconCfg->olimage)
  {
    RECT r;
    GetPositionPaintOverExtent(&r);

    int sx=0;
    int sy=0;
    int w=m_iconCfg->olimage->getWidth();
    int h=m_iconCfg->olimage->getHeight();
    if (m_iconCfg->image_ltrb_used)  { w-=2; h-= 2; sx++,sy++; }

    w/=3;

    if (w>0 && h>0)
    {
      if (m_is_button)
      {
        if ((m_pressed&2))  sx+=(m_pressed&1) ? w*2 : w;
      }
      LICE_ScaledBlit(drawbm,m_iconCfg->olimage,r.left+origin_x,r.top+origin_y,
        r.right-r.left,
        r.bottom-r.top,
        (float)sx,(float)sy,(float)w,(float)h, m_alpha,  // m_grayed?
        LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR|LICE_BLIT_USE_ALPHA);      
    }
  }
}


void WDL_VirtualIconButton::OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect) 
{ 
  HDC hdc=drawbm->getDC();
  int col;

  float alpha = (m_grayed ? 0.25f : 1.0f) * m_alpha;

  bool isdown = !!(m_pressed&1);
  bool ishover = !!(m_pressed&2);

  if (m_iconCfg && m_iconCfg->image && !m_iconCfg->image_issingle)
  {
    bool swapupdown = (m_checkstate > 0);
    bool isdownimg = (swapupdown != isdown);
    
    RECT r=m_position;

    int sx=0;
    int sy=0;
    int w=m_iconCfg->image->getWidth();
    int h=m_iconCfg->image->getHeight();

    w/=3;
    if (w>0 && h > 0)
    {
      if (m_is_button)
      {
        if (isdownimg) sx += w*2;
        else if (ishover) sx += w;
      }

      LICE_ScaledBlit(drawbm,m_iconCfg->image,r.left+origin_x,r.top+origin_y,
        r.right-r.left,
        r.bottom-r.top,
        (float)sx,(float)sy,(float)w,(float)h, alpha,
        LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR|LICE_BLIT_USE_ALPHA);      
    }
  }
  else
  {
    RECT r=m_position;
    r.left+=origin_x;
    r.right+=origin_x;
    r.top+=origin_y;
    r.bottom+=origin_y;
    if (m_is_button)
    {
      if (WDL_STYLE_WantGlobalButtonBackground(&col))
      {
        LICE_FillRect(drawbm,r.left,r.top,r.right-r.left,r.bottom-r.top,LICE_RGBA_FROMNATIVE(col,255),alpha,LICE_BLIT_MODE_COPY);
      }

      if (ishover || m_forceborder || WDL_STYLE_WantGlobalButtonBorders())
      {
        int cidx=isdown?COLOR_3DSHADOW:COLOR_3DHILIGHT;

        int pencol = WDL_STYLE_GetSysColor(cidx);
        pencol = LICE_RGBA_FROMNATIVE(pencol,255);

        LICE_Line(drawbm,r.left,r.bottom-1,r.left,r.top,pencol,alpha,LICE_BLIT_MODE_COPY,false);
        LICE_Line(drawbm,r.left,r.top,r.right-1,r.top,pencol,alpha,LICE_BLIT_MODE_COPY,false);
        cidx = isdown?COLOR_3DHILIGHT:COLOR_3DSHADOW;
        pencol = WDL_STYLE_GetSysColor(cidx);
        pencol = LICE_RGBA_FROMNATIVE(pencol,255);
        LICE_Line(drawbm,r.right-1,r.top,r.right-1,r.bottom-1,pencol,alpha,LICE_BLIT_MODE_COPY,false);
        LICE_Line(drawbm,r.right-1,r.bottom-1,r.left,r.bottom-1,pencol,alpha,LICE_BLIT_MODE_COPY,false);
      }
    }
    if (m_iconCfg && m_iconCfg->image)
    {
      int sz=16,sz2=16;
      WDL_STYLE_ScaleImageCoords(&sz,&sz2);

      //if (m_position.right-m_position.left > 24) sz=m_position.right-m_position.left-8;
    
      int x=r.left+((r.right-r.left)-sz)/2;
      int y=r.top+((r.bottom-r.top)-sz2)/2;
      if (m_is_button)
      {
        if (isdown && ishover) { x++; y++; }
      }

      LICE_ScaledBlit(drawbm,m_iconCfg->image,x,y,sz,sz2,0.0f,0.0f,
        (float)m_iconCfg->image->getWidth(),
        (float)m_iconCfg->image->getHeight(),alpha,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR|LICE_BLIT_USE_ALPHA);

    }
  }

  if (!m_iconCfg || m_forcetext)
  {
    RECT r2=m_position;
    r2.left+=origin_x;
    r2.right+=origin_x;
    r2.top+=origin_y;
    r2.bottom+=origin_y;

    if (m_checkstate>=0 && !m_iconCfg)
    {
      RECT tr=r2;
      int sz=tr.bottom-tr.top;
      r2.left+=sz+2;

      tr.top+=2;
      tr.bottom-=2;
      sz-=4;
      sz&=~1;
      LICE_FillRect(drawbm ,tr.left,tr.top,sz,sz,LICE_RGBA(255,255,255,255),alpha,LICE_BLIT_MODE_COPY);
      LICE_Line(drawbm,tr.left,tr.top,tr.left+sz,tr.top,LICE_RGBA(128,128,128,255),alpha,LICE_BLIT_MODE_COPY,false);
      LICE_Line(drawbm,tr.left+sz,tr.top,tr.left+sz,tr.bottom,LICE_RGBA(128,128,128,255),alpha,LICE_BLIT_MODE_COPY,false);
      LICE_Line(drawbm,tr.left+sz,tr.bottom,tr.left,tr.bottom,LICE_RGBA(128,128,128,255),alpha,LICE_BLIT_MODE_COPY,false);
      LICE_Line(drawbm,tr.left,tr.bottom,tr.left,tr.top,LICE_RGBA(128,128,128,255),alpha,LICE_BLIT_MODE_COPY,false);
      int nl = (m_checkstate>0) ? 3:0;        
      if (isdown) nl ^= 2;

      if (nl&1)
        LICE_Line(drawbm,tr.left+2,tr.bottom-2,tr.left+sz-2,tr.top+2,LICE_RGBA(0,0,0,255),alpha,LICE_BLIT_MODE_COPY,false);
      if (nl&2)
        LICE_Line(drawbm,tr.left+2,tr.top+2,tr.left+sz-2,tr.bottom-2,LICE_RGBA(0,0,0,255),alpha,LICE_BLIT_MODE_COPY,false);


    }
    // draw text
    if (m_textfont&&m_textlbl.Get()[0])
    {
      int fgc=WDL_STYLE_GetSysColor(COLOR_BTNTEXT);
      fgc=LICE_RGBA_FROMNATIVE(fgc,255);
      //m_textfont->SetCombineMode(LICE_BLIT_MODE_COPY, alpha); // this affects the glyphs that get cached
      m_textfont->SetBkMode(TRANSPARENT);
      m_textfont->SetTextColor(fgc);

      r2.left += m_margin_l;
      r2.right -= m_margin_r;
      r2.top += m_margin_t;
      r2.bottom -= m_margin_b;

      if (isdown)
      {
        if (m_textalign<0) r2.left+=1;
        else if (m_textalign>0) r2.right+=1;
        else r2.left+=2;
        r2.top+=2;
      }
      m_textfont->DrawText(drawbm,m_textlbl.Get(),-1,&r2,DT_SINGLELINE|DT_VCENTER|(m_textalign<0?DT_LEFT:m_textalign>0?DT_RIGHT:DT_CENTER)|DT_NOPREFIX);
    }
    
  }

  if (m_bgcol1_msg)
  {
    int brcol=-100;
    SendCommand(m_bgcol1_msg,(INT_PTR)&brcol,GetID(),this);
    if (brcol != -100)
    {
      RECT r=m_position;

      int bh=(r.bottom-r.top)/5;
      if (bh<1) bh=1;
      int bw=(r.right-r.left)/5;
      if (bw<1) bw=1;

      LICE_FillRect(drawbm,
        r.left+origin_x,r.top+origin_y,
        r.right-r.left,
        bh,LICE_RGBA_FROMNATIVE(brcol,255),0.75,LICE_BLIT_MODE_COPY);

      LICE_FillRect(drawbm,
        r.left+origin_x,r.top+origin_y+bh,
        bw,
        r.bottom-r.top-bh*2,LICE_RGBA_FROMNATIVE(brcol,255),0.75,LICE_BLIT_MODE_COPY);

      LICE_FillRect(drawbm,
        r.right+origin_x-bw,r.top+origin_y+bh,
        bw,
        r.bottom-r.top-bh*2,LICE_RGBA_FROMNATIVE(brcol,255),0.75,LICE_BLIT_MODE_COPY);

      LICE_FillRect(drawbm,
        r.left+origin_x,r.bottom+origin_y-bh,
        r.right-r.left,
        bh,LICE_RGBA_FROMNATIVE(brcol,255),0.75,LICE_BLIT_MODE_COPY);
    }
  }

} 


void WDL_VirtualIconButton::OnMouseMove(int xpos, int ypos)
{
  if (m_en&&m_is_button)
  {
    int wp=m_pressed;
    if (xpos >= 0 &&
      xpos < m_position.right-m_position.left && 
      ypos >= 0 &&
      ypos < m_position.bottom-m_position.top)
    {
      m_pressed|=2;
    }
    else
    {
      m_pressed&=~2;
    }

    if ((m_pressed&3)!=(wp&3))
    {
      RequestRedraw(NULL);
    }
  }
}

int WDL_VirtualIconButton::OnMouseDown(int xpos, int ypos)
{
  if (m_en&&m_is_button)
  {
    m_pressed=3;
    RequestRedraw(NULL);

    if (m_immediate)
    {
      DoSendCommand(xpos, ypos);
    }

    return 1;
  }
  return 0;
}

bool WDL_VirtualIconButton::OnMouseDblClick(int xpos, int ypos)
{
  if (m_is_button) 
  { 
    DoSendCommand(xpos, ypos);
    return true;
  }
  return false;
}

void WDL_VirtualIconButton::OnMouseUp(int xpos, int ypos)
{
  if (!m_is_button) return;

  int waspress=!!m_pressed;
  m_pressed&=~1;
  RequestRedraw(NULL);

  if (waspress && !m_immediate)
  {
    DoSendCommand(xpos, ypos);
  }
}

void WDL_VirtualIconButton::DoSendCommand(int xpos, int ypos)
{
  if (m_en &&
    xpos >= 0 &&
    xpos < m_position.right-m_position.left && 
    ypos >= 0 && 
    ypos < m_position.bottom-m_position.top)
  {
    int code=GetID();
    if (!m_iconCfg && m_textlbl.Get()[0] && m_checkstate >= 0)
    {
      if (xpos < m_position.bottom-m_position.top)
      {
        code|=600<<16;
      }
    }
    SendCommand(WM_COMMAND,code,0,this);
  }
}


WDL_VirtualComboBox::WDL_VirtualComboBox()
{
  m_font=0;
  m_align=-1;
  m_curitem=-1;
}

WDL_VirtualComboBox::~WDL_VirtualComboBox()
{
  m_items.Empty(true,free);
}


static void GenSubMenu(HMENU menu, int *x, WDL_PtrList<char> *items, int curitem)
{
  int pos=0;
  while (*x < items->GetSize())
  {
    MENUITEMINFO mi={sizeof(mi),MIIM_ID|MIIM_STATE|MIIM_TYPE,MFT_STRING, 0,1000+*x,NULL,NULL,NULL,0};
    mi.dwTypeData = (char *)items->Get(*x);
    mi.fState = curitem == *x ?MFS_CHECKED:0;

    (*x) ++; // advance to next item

    if (!strcmp(mi.dwTypeData,"<SEP>")) mi.fType=MFT_SEPARATOR;
    else if (!strcmp(mi.dwTypeData,"</SUB>")) break; // done!
    else if (!strncmp(mi.dwTypeData,"<SUB>",5))
    {
      mi.hSubMenu= CreatePopupMenu();
      GenSubMenu(mi.hSubMenu,x,items,curitem);
      mi.fMask |= MIIM_SUBMENU;
      mi.dwTypeData += 5; // skip <SUB>
    }
    InsertMenuItem(menu,pos++,TRUE,&mi);
  }
}

int WDL_VirtualComboBox::OnMouseDown(int xpos, int ypos)
{
  if (m_items.GetSize())
  {    
    //SendCommand(WM_COMMAND, GetID()|(CBN_DROPDOWN<<16), 0, this);

    HMENU menu=CreatePopupMenu();
    int x=0;
    GenSubMenu(menu,&x,&m_items,m_curitem);

    HWND h=GetRealParent();
    POINT p={0,};
    WDL_VirtualWnd *w=this;
    while (w)
    {
      RECT r;
      w->GetPosition(&r);
      p.x+=r.left;
      p.y+=w==this?r.bottom:r.top;
      w=w->GetParent();
    }
    if (h) 
    {
      ClientToScreen(h,&p);
      //SetFocus(h);
    }
    
    int ret=TrackPopupMenu(menu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_NONOTIFY,p.x,p.y,0,h,NULL);

    if (ret>=1000)
    {
      m_curitem=ret-1000;
      RequestRedraw(NULL);
    // track menu
      SendCommand(WM_COMMAND,GetID() | (CBN_SELCHANGE<<16),0,this);
    }
  }
  return -1;
}

void WDL_VirtualComboBox::OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect)
{
  {
    if (m_font) m_font->SetBkMode(TRANSPARENT);

    RECT r=m_position;
    r.left+=origin_x;
    r.right+=origin_x;
    r.top+=origin_y;
    r.bottom+=origin_y;

    int col=WDL_STYLE_GetSysColor(COLOR_WINDOW);
    col = LICE_RGBA_FROMNATIVE(col,255);
    LICE_FillRect(drawbm,r.left,r.top,r.right-r.left,r.bottom-r.top,col,1.0f,LICE_BLIT_MODE_COPY);

    {
      RECT tr=r;
      tr.left=tr.right-(tr.bottom-tr.top);
      int col2=WDL_STYLE_GetSysColor(COLOR_BTNFACE);
      col2 = LICE_RGBA_FROMNATIVE(col2,255);

      LICE_FillRect(drawbm,tr.left,tr.top,tr.right-tr.left,tr.bottom-tr.top,col,1.0f,LICE_BLIT_MODE_COPY);
    }
    

    int tcol=WDL_STYLE_GetSysColor(COLOR_BTNTEXT);
    tcol=LICE_RGBA_FROMNATIVE(tcol,255);
    if (m_font && m_items.Get(m_curitem)&&m_items.Get(m_curitem)[0])
    {
      RECT tr=r;
      tr.left+=2;
      tr.right-=16;
      m_font->SetTextColor(tcol);
      m_font->DrawText(drawbm,m_items.Get(m_curitem),-1,&tr,DT_SINGLELINE|DT_VCENTER|(m_align<0?DT_LEFT:m_align>0?DT_RIGHT:DT_CENTER)|DT_NOPREFIX);
    }


    // pen3=tcol
    int pencol = WDL_STYLE_GetSysColor(COLOR_3DSHADOW);
    pencol = LICE_RGBA_FROMNATIVE(pencol,255);
    int pencol2 = WDL_STYLE_GetSysColor(COLOR_3DHILIGHT);
    pencol2 = LICE_RGBA_FROMNATIVE(pencol2,255);

    // draw the down arrow button
    {
      int bs=(r.bottom-r.top);
      int l=r.right-bs;

      int a=(bs/4)&~1;

      LICE_Line(drawbm,l,r.top,l,r.bottom-1,pencol,1.0f,LICE_BLIT_MODE_COPY,false);
      LICE_Line(drawbm,l-1,r.top,l-1,r.bottom-1,pencol2,1.0f,LICE_BLIT_MODE_COPY,false);

      LICE_Line(drawbm,l+bs/2-a,r.top+bs/2-a/2,
                       l+bs/2,r.top+bs/2+a/2,tcol,1.0f,LICE_BLIT_MODE_COPY,true);
      LICE_Line(drawbm,l+bs/2,r.top+bs/2+a/2,
                       l+bs/2+a,r.top+bs/2-a/2, tcol,1.0f,LICE_BLIT_MODE_COPY,true);
    }

   

    // draw the border
    LICE_Line(drawbm,r.left,r.bottom-1,r.left,r.top,pencol,1.0f,0,false);
    LICE_Line(drawbm,r.left,r.top,r.right-1,r.top,pencol,1.0f,0,false);
    LICE_Line(drawbm,r.right-1,r.top,r.right-1,r.bottom-1,pencol2,1.0f,0,false);
    LICE_Line(drawbm,r.left,r.bottom-1,r.right-1,r.bottom-1,pencol2,1.0f,0,false);

  }
}



WDL_VirtualStaticText::WDL_VirtualStaticText()
{
  m_dotint=false;
  m_bkbm=0;
  m_margin_r=m_margin_l=0;
  m_margin_t=m_margin_b=0;
  m_fg=m_bg=-1;
  m_wantborder=false;
  m_vfont=m_font=0;
  m_align=-1;
  m_wantsingle=false;
  m_didvert=0;
  m_didalign=-1;
}

WDL_VirtualStaticText::~WDL_VirtualStaticText()
{
}

void WDL_VirtualStaticText::SetText(const char *text) 
{ 
  if (strcmp(m_text.Get(),text))
  {
    m_text.Set(text); 
    if (m_font) RequestRedraw(NULL); 
  }
}

int WDL_VirtualStaticText::OnMouseDown(int xpos, int ypos)
{
  int a = WDL_VWnd::OnMouseDown(xpos,ypos);
  if (a) return a;

  if (m_wantsingle)
  {
    SendCommand(WM_COMMAND,GetID() | (STN_CLICKED<<16),0,this);
    return -1;
  }
  return 0;
}

void WDL_VirtualStaticText::OnPaint(LICE_IBitmap *drawbm, int origin_x, int origin_y, RECT *cliprect)
{
  RECT r=m_position;
  r.left+=origin_x;
  r.right+=origin_x;
  r.top += origin_y;
  r.bottom += origin_y;

  if (m_bkbm && m_bkbm->bgimage)
  {
    WDL_VirtualWnd_ScaledBlitBG(drawbm,m_bkbm,
      r.left,r.top,r.right-r.left,r.bottom-r.top,
      r.left,r.top,r.right-r.left,r.bottom-r.top,
      1.0,LICE_BLIT_MODE_COPY|LICE_BLIT_FILTER_BILINEAR|LICE_BLIT_USE_ALPHA);

    if (m_dotint && m_bg>=0) 
    {
        float rv=GetRValue(m_bg)/255.0f;
        float gv=GetGValue(m_bg)/255.0f;
        float bv=GetBValue(m_bg)/255.0f;

        float avg=(rv+gv+bv)*0.33333f;
        if (avg<0.05f)avg=0.05f;

        float sc=0.5f;
        float sc2 = (1.0f-sc)/avg;

        float sc3=32.0f;
        float sc4=64.0f*(avg-0.5f);
        // tint
        LICE_MultiplyAddRect(drawbm,
          r.left,r.top,
            r.right-r.left,
            r.bottom-r.top,
            sc+rv*sc2,sc+gv*sc2,sc+bv*sc2,1,
            (rv-avg)*sc3+sc4,
            (gv-avg)*sc3+sc4,
            (bv-avg)*sc3+sc4,
            0);
    }
  }
  else 
  {
    if (m_bg!=-1)
    {
      LICE_FillRect(drawbm,r.left,r.top,r.right-r.left,r.bottom-r.top,LICE_RGBA_FROMNATIVE(m_bg,255),1.0f,LICE_BLIT_MODE_COPY);
    }

    if (m_wantborder)
    {    
      int cidx=COLOR_3DSHADOW;

      int pencol = WDL_STYLE_GetSysColor(cidx);
      pencol = LICE_RGBA_FROMNATIVE(pencol,255);

      LICE_Line(drawbm,r.left,r.bottom-1,r.left,r.top,pencol,1.0f,LICE_BLIT_MODE_COPY,false);
      LICE_Line(drawbm,r.left,r.top,r.right-1,r.top,pencol,1.0f,LICE_BLIT_MODE_COPY,false);
      cidx=COLOR_3DHILIGHT;
      pencol = WDL_STYLE_GetSysColor(cidx);
      pencol = LICE_RGBA_FROMNATIVE(pencol,255);
      LICE_Line(drawbm,r.right-1,r.top,r.right-1,r.bottom-1,pencol,1.0f,LICE_BLIT_MODE_COPY,false);
      LICE_Line(drawbm,r.right-1,r.bottom-1,r.left,r.bottom-1,pencol,1.0f,LICE_BLIT_MODE_COPY,false);

      r.left++;
      r.bottom--;
      r.top++;
      r.right--;

    }
  }


  if (m_text.Get()[0])
  {

    r.left += m_margin_l;
    r.right -= m_margin_r;
    r.top += m_margin_t;
    r.bottom -= m_margin_b;

    m_didvert=m_vfont && (r.right-r.left)<(r.bottom-r.top);
    LICE_IFont *font = m_didvert ? m_vfont : m_font;


    if (font)
    {
      font->SetBkMode(TRANSPARENT);

    
      m_didalign=m_align;
      if (m_didalign==0)
      {
        RECT r2={0,0,0,0};
        font->DrawText(drawbm,m_text.Get(),-1,&r2,DT_SINGLELINE|DT_VCENTER|DT_LEFT|DT_NOPREFIX|DT_CALCRECT);
        if (r2.right > r.right-r.left) m_didalign=-1;
      }

      int tcol=m_fg!=-1 ? m_fg : LICE_RGBA_FROMNATIVE(WDL_STYLE_GetSysColor(COLOR_BTNTEXT));
      font->SetTextColor(tcol);
      font->DrawText(drawbm,m_text.Get(),-1,&r,DT_SINGLELINE|DT_VCENTER|(m_didalign<0?DT_LEFT:m_didalign>0?DT_RIGHT:DT_CENTER)|DT_NOPREFIX);
    }


  }
  WDL_VWnd::OnPaint(drawbm,origin_x,origin_y,cliprect);
}


int WDL_VirtualStaticText::GetCharFromCoord(LICE_IBitmap* bmp, int xpos, int ypos)
{
  LICE_IFont *font = (m_didvert ? m_vfont : m_font);
  if (!font) return -1;
  
  const char* str = m_text.Get();
  int len = strlen(str);
  if (!len) return -1;

  // for align left/right, we could DT_CALCRECT with 1 char, then 2, etc, but that won't work for align center
  // so we'll just estimate
  RECT tr;
  font->DrawText(bmp, str, len, &tr, DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_CALCRECT);
  int tw = tr.right;
  int th = tr.bottom;

  RECT r = m_position;
  if (m_wantborder)
  {
    r.left++;
    r.top++;
    r.right--;
    r.bottom--;
  }
  r.left += m_margin_l;
  r.top += m_margin_t;
  r.right -= m_margin_r;
  r.bottom -= m_margin_b;
  int w = r.right-r.left;
  int h = r.bottom-r.top;

  if (m_didvert)
  {
    r.left += (w-tw)/2;
    r.right -= (w-tw)/2;
  }
  else
  {
    r.top += (h-th)/2;
    r.bottom -= (h-th)/2;
  }

  if (m_didalign < 0)
  {
    if (m_didvert) r.bottom = r.top+th;    
    else r.right = r.left+tw;    
  }
  else if (m_didalign > 0)
  {
    if (m_didvert) r.top = r.bottom-th;
    else r.left = r.right-tw;
  }
  else
  {
    if (m_didvert) 
    {
      r.top += (h-th)/2;
      r.bottom -= (h-th)/2;
    }
    else
    {
      r.left += (w-tw)/2;
      r.right -= (w-tw)/2;
    }
  }

  int c=-1;
  if (m_didvert)
  {
    if (ypos < r.top) c=-1;
    else if (ypos > r.bottom) c=len;
    else c = (int)((double)len*(double)(ypos-r.top)/(double)(r.bottom-r.top));
  }
  else
  {
    if (xpos < r.left) c=len;
    else if (xpos > r.right) c=-1;
    else c = (int)((double)len*(double)(xpos-r.left)/(double)(r.right-r.left));
  }

  return c;
}


bool WDL_VirtualStaticText::OnMouseDblClick(int xpos, int ypos)
{
  if (!WDL_VWnd::OnMouseDblClick(xpos,ypos))
  {
    SendCommand(WM_COMMAND,GetID() | (STN_DBLCLK<<16),0,this);
  }

  return true;
}


bool WDL_VirtualIconButton::WantsPaintOver()
{
  return m_is_button && m_iconCfg && m_iconCfg->image && m_iconCfg->olimage;
}

void WDL_VirtualIconButton::GetPositionPaintOverExtent(RECT *r)
{
  *r=m_position;
  if (m_iconCfg && m_iconCfg->image && m_iconCfg->olimage && m_iconCfg->image_ltrb_used)
  {
    int w=(m_iconCfg->olimage->getWidth()-2)/3-m_iconCfg->image_ltrb[0]-m_iconCfg->image_ltrb[2];
    if (w<1)w=1;
    double wsc=(r->right-r->left)/(double)w;

    int h=m_iconCfg->olimage->getHeight()-2-m_iconCfg->image_ltrb[1]-m_iconCfg->image_ltrb[3];
    if (h<1)h=1;
    double hsc=(r->bottom-r->top)/(double)h;

    r->left-=(int) (m_iconCfg->image_ltrb[0]*wsc);
    r->top-=(int) (m_iconCfg->image_ltrb[1]*hsc);
    r->right+=(int) (m_iconCfg->image_ltrb[2]*wsc);
    r->bottom+=(int) (m_iconCfg->image_ltrb[3]*hsc);
  }
}
void WDL_VirtualIconButton_PreprocessSkinConfig(WDL_VirtualIconButton_SkinConfig *a)
{
  if (a && a->image && a->olimage)
  {
    a->image_ltrb_used=false;
    int lext=0,rext=0,bext=0,text=0;

    int w=a->olimage->getWidth();
    int h=a->olimage->getHeight();

    if (LICE_GetPixel(a->olimage,0,0)==LICE_RGBA(255,0,255,255)&&
        LICE_GetPixel(a->olimage,w-1,h-1)==LICE_RGBA(255,0,255,255))
    {
      int x;
      for (x = 1; x < w/3 && LICE_GetPixel(a->olimage,x,0)==LICE_RGBA(255,0,255,255); x ++);
      lext=x-1;
      for (x = 1; x < h && LICE_GetPixel(a->olimage,0,x)==LICE_RGBA(255,0,255,255); x ++);
      text=x-1;

      for (x = w-2; x > (w*2/3) && LICE_GetPixel(a->olimage,x,h-1)==LICE_RGBA(255,0,255,255); x --);
      rext=w-2-x;
      for (x = h-2; x > text && LICE_GetPixel(a->olimage,w-1,x)==LICE_RGBA(255,0,255,255); x --);
      bext=h-2-x;
      if (lext||text||rext||bext)
      {
        a->image_ltrb_used=true;
        a->image_ltrb[0]=lext;
        a->image_ltrb[1]=text;
        a->image_ltrb[2]=rext;
        a->image_ltrb[3]=bext;
      }
    }

  }
}