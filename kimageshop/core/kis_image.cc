/*
 *  kis_image.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Andrew Richards <A.Richards@phys.canterbury.ac.nz>
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <stdio.h>
#include <iostream.h>
#include <sys/time.h>
#include <unistd.h>

#include <qpainter.h>
#include <qregexp.h>
#include <qfileinfo.h>

#include <kapp.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <kmimetype.h>
#include <kdebug.h>

#include "kis_factory.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_util.h"
#include "kis_brush.h"
#include "kis_global.h"
#include "kis_util.h"
#include "kis_timer.h"

#define KIS_DEBUG(AREA, CMD)

KisImage::KisImage( const QString& n, int w, int h, cMode cm, uchar bd )
  : m_name (n)
  , m_width (w)
  , m_height (h)
  , m_cMode (cm)
  , m_bitDepth (bd)
{
    m_author = "unknown";
    m_email = "unknown";

    QRect tileExtents = KisUtil::findTileExtents( QRect(0, 0, m_width, m_height) );
  
    m_xTiles = tileExtents.width() / TILE_SIZE;	
    m_yTiles = tileExtents.height() / TILE_SIZE;

    // setup dirty flag array
    m_dirty.resize (m_xTiles * m_yTiles);
    
    for(int y = 0; y < m_yTiles; y++)
        for(int x = 0; x < m_xTiles; x++)
	    m_dirty[y * m_xTiles + x] = false;

    setUpVisual();
    QPixmap::setDefaultOptimization( QPixmap::NoOptim );

    m_ptiles = new QPixmap* [m_xTiles*m_yTiles];

    for( int y = 0; y < m_yTiles; y++)
        for( int x = 0; x < m_xTiles; x++) 
        {
            m_ptiles[y * m_xTiles + x] = new QPixmap(TILE_SIZE, TILE_SIZE);
            m_ptiles[y * m_xTiles + x]->fill();
        }
  
    m_img.create(TILE_SIZE, TILE_SIZE, 32);
  
    m_pCurrentLay = 0;

    m_pComposeLay = new KisLayer( "_compose", m_cMode, m_bitDepth );
    m_pComposeLay->allocateRect( QRect( 0, 0, TILE_SIZE, TILE_SIZE ) );
  
    m_pBGLay = new KisLayer( "_background", m_cMode, m_bitDepth );
    m_pBGLay->allocateRect( QRect( 0, 0, TILE_SIZE, TILE_SIZE ) );

    // FIXME: make it work with non-RGB color spaces
    
    uchar* ptr0 = m_pBGLay->channelMem(0, 0, 0, 0); // red
    uchar* ptr1 = m_pBGLay->channelMem(1, 0, 0, 0); // green
    uchar* ptr2 = m_pBGLay->channelMem(2, 0, 0, 0); // blue
    uchar* ptr3 =0;                                 // appha    

    if (m_cMode == cm_RGBA)
	ptr3 = m_pBGLay->channelMem(3, 0, 0, 0);

    for( int y = 0; y < TILE_SIZE; y++)
        for(int x = 0; x < TILE_SIZE; x++)
	{
	    uchar v = 128+63*((x/16+y/16)%2);
	    *(ptr0 + (y * TILE_SIZE + x)) = v;
	    *(ptr1 + (y * TILE_SIZE + x)) = v;
	    *(ptr2 + (y * TILE_SIZE + x)) = v;
	    if (m_cMode == cm_RGBA)
		  *(ptr3 + (y * TILE_SIZE + x)) = 255;
	}

    compositeImage(QRect()); 

    /* note - 32 bit pixmaps may not show up on 16 bit display and will
     crash it !!!  Without depth paramater will use native format
    imagePixmap = new QPixmap(w, h, 32); */

#ifdef JOHN
    imagePixmap = new QPixmap(w, h);
    imagePixmap->fill();
#endif
    
    // start update timer
    m_pUpdateTimer = new QTimer(this);
    connect( m_pUpdateTimer, SIGNAL(timeout()), this, SLOT(slotUpdateTimeOut()) );
    m_pUpdateTimer->start(1);
}

KisImage::~KisImage()
{
    qDebug("~KisImage()");
    
#ifdef JOHN
    delete imagePixmap;
#endif

    for( int y = 0; y < m_yTiles; y++)
        for( int x = 0; x < m_xTiles; x++)
	    delete m_ptiles[y * m_xTiles + x];

    delete m_ptiles;

    if ((visual!=unknown) && (visual!=rgb888x))
        free(m_pImgData);
        
}

void KisImage::markDirty( QRect r )
{
    for(int y = 0; y < m_yTiles; y++)
        for(int x = 0; x < m_xTiles; x++)
            if (r.intersects(QRect(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE)))
		m_dirty[y * m_xTiles + x] = true;
}


void KisImage::slotUpdateTimeOut()
{
    for(int y = 0; y < m_yTiles; y++)
	for(int x = 0; x < m_xTiles; x++)
	    if (m_dirty[y * m_xTiles + x])
	        compositeImage(QRect(x*TILE_SIZE, y*TILE_SIZE,TILE_SIZE,TILE_SIZE));

    for(int y = 0; y < m_yTiles; y++)
        for(int x = 0; x < m_xTiles; x++)
	    if (m_dirty[y * m_xTiles + x])
	    {
	        m_dirty[y * m_xTiles + x] = false;
		emit updated(QRect(x*TILE_SIZE, y*TILE_SIZE,TILE_SIZE,TILE_SIZE));
	    }
}


void KisImage::paintContent( QPainter& painter, 
    const QRect& rect, bool /*transparent*/)
{
    paintPixmap( &painter, rect);
}



void KisImage::paintPixmap(QPainter *p, QRect area)
{
    if (!p) return;
    
    // KisTimer::start();
    int startX, startY, pixX, pixY, clipX, clipY = 0;

    int l = area.left();
    int t = area.top();
    int r = area.right();
    int b = area.bottom();
    
#if 0
    p->drawPixmap(l, t,     /* offset into destination */
        *imagePixmap,       /* pixmap - not ptr */
        l, t,               /* offset into source */
        r-l, b - t          /* amount of source to copy */  
    );
#endif

    //kdDebug(0) << "#### KisImage::paintPixmap() ####" << endl;
    //qDebug("KisImage::paintPixmap l: %d; t: %d; r: %d; b: %d", l, t, r, b);
 
    for(int y=0; y < m_yTiles; y++)
    {
        for(int x=0; x < m_xTiles; x++)
	{
            QRect tileRect(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE);

	    int xt = x*TILE_SIZE;
	    int yt = y*TILE_SIZE;
	    //qDebug("tile: %d", y*m_xTiles+x);

	    if (tileRect.intersects(QRect(0, 0, m_width, m_height)) 
                && tileRect.intersects(area))
	    {
	        if (m_ptiles[y*m_xTiles+x]->isNull())
		    continue;
	      
	        if (xt < l)
		{
		    startX = 0;
		    pixX = l - xt;
		}
	        else
		{
		    startX = xt - l;
		    pixX = 0;
		}

	        clipX = r - xt - pixX;
	      
	        if (yt < t)
		{
		    startY = 0;
		    pixY = t - yt;
		}
	        else
		{
		    startY = yt - t;
		    pixY = 0;
		}
	        clipY = b - yt - pixY;

	        if (clipX <= 0)
		    clipX = -1;
	        if (clipY <= 0)
		    clipY = -1;

	        //qDebug("clipX %d clipY %d", clipX, clipY);
	        //qDebug("pixX %d pixY %d", pixX, pixY);
	      
	        p->drawPixmap(startX, startY, *m_ptiles[y*m_xTiles+x], pixX, pixY, clipX, clipY);
                //impPainter->drawPixmap(startX, startY, *m_ptiles[y*m_xTiles+x], pixX, pixY, clipX, clipY);
	    }     
	}
    }
    
    // KisTimer::stop("paintImage ");
}



void KisImage::setUpVisual()
{

    QPixmap p;
    Display *dpy    =   p.x11Display();
    int displayDepth=   p.x11Depth();
    Visual *vis     =   (Visual*)p.x11Visual();
    bool trueColour =   (vis->c_class == TrueColor);

    // change the false to true to test the faster image converters
    visual = unknown;
    
    if (true && trueColour) // do they have a worthy display
    { 
        uint red_mask  =(uint)vis->red_mask;
        uint green_mask=(uint)vis->green_mask;
        uint blue_mask =(uint)vis->blue_mask;

        if ((red_mask==0xf800) && (green_mask==0x7e0) && (blue_mask==0x1f))
            visual=rgb565;
        if ((red_mask==0xff0000) && (green_mask==0xff00) && (blue_mask==0xff))
            visual=rgb888x;

        if (visual==unknown) 
        {
            puts("Unoptimized visual - want to write an optimised routine?");
            printf("red=%8x green=%8x blue=%8x\n",red_mask,green_mask,blue_mask);
        } 
        else 
        {
            puts("Using optimized visual");
            m_pxi=XCreateImage( dpy, vis, displayDepth, ZPixmap, 0,0, TILE_SIZE,TILE_SIZE, 32, 0 );
            printf("ximage: bytes_per_line=%d\n",m_pxi->bytes_per_line);
            if (visual!=rgb888x) 
            {
		m_pImgData=new char[m_pxi->bytes_per_line*TILE_SIZE];
		m_pxi->data=m_pImgData;
            }
        }
    }
}	


void KisImage::addLayer(const QRect& rect, const KisColor& c, 
    bool tr, const QString& name)
{
kdDebug(0) << "KisImage::addLayer() : entering" << endl; 
    KisLayer *lay = new KisLayer(name, m_cMode, m_bitDepth);

#ifndef JOHN

    lay->allocateRect(rect);
kdDebug(0) << "KisImage::addLayer() : returned from lay->allocateRect()" << endl;         
    lay->clear(c, tr);
kdDebug(0) << "KisImage::addLayer() : returned from lay->clear()" << endl;         

#endif
    
kdDebug(0) << "KisImage::addLayer() : returned from lay->claer()" << endl;         
    m_layers.append(lay);
kdDebug(0) << "KisImage::addLayer() : returned from m_layers->append()" << endl;             
    m_pCurrentLay=lay;
kdDebug(0) << "KisImage::addLayer() : leaving" << endl;     
}


void KisImage::removeLayer( unsigned int _layer )
{
    if( _layer >= m_layers.count() )
        return;

    KisLayer* lay = m_layers.take( _layer );

    if( m_pCurrentLay == lay )
    {
        if( m_layers.count() != 0 )
            m_pCurrentLay = m_layers.at( 0 );
        else
            m_pCurrentLay = NULL;
    }

  delete lay;
}


/* 
    Constructs the composite image in the tile at x,y 
    and updates the relevant pixmap
*/
    
void KisImage::compositeTile(int x, int y, KisLayer *dstLay, int dstTile)
{
    // work out which tile to render into unless directed to a specific tile
    if (dstTile==-1)
        dstTile=y*m_xTiles+x;
    if (dstLay==0)
        dstLay=m_pComposeLay;

    KIS_DEBUG(tile, printf("\n*** compositeTile %d,%d\n",x,y); );

    //printf("compositeTile: dstLay=%p dstTile=%d\n",dstLay, dstTile);

    // Set the background
    for (uchar i = 0; i < dstLay->numChannels(); i++)
  	memcpy(dstLay->channelMem(i, dstTile, 0, 0), 
            m_pBGLay->channelMem(i, dstTile, 0, 0), TILE_SIZE * TILE_SIZE);

    // Find the tiles boundary in KisImage coordinates
    QRect tileBoundary(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE);

    int l=0;
    KisLayer *lay=m_layers.first();
    
    while(lay) 
    { // Go through each layer and find its contribution to this tile
        l++;
        //printf("layer: %s opacity=%d\n",lay->name().data(), lay->opacity());
        if ((lay->visible()) &&
	(tileBoundary.intersects(lay->imageExtents()))) 
        {
        // The layer is part of the tile. Find out the 1-4 tiles of the channel
        // which are in it and render the appropriate proportions of each
        //TIME_START;
        //printf("*** compositeTile %d,%d\n",x,y);
        renderLayerIntoTile(tileBoundary, lay, dstLay, dstTile);
        //TIME_END("renderLayerIntoTile");
        }
        lay=m_layers.next();
    }
}



void KisImage::compositeImage(QRect r)
{
    //KisTimer::start();

    for(int y = 0; y < m_yTiles; y++)
        for(int x = 0; x < m_xTiles; x++)
            if (r.isNull() || r.intersects(QRect(x*TILE_SIZE, y*TILE_SIZE,TILE_SIZE,TILE_SIZE)))
	    {
		  if (m_cMode == cm_RGBA) // set the alpha channel to opaque
			memset(m_pComposeLay->channelMem(3, 0, 0, 0),255, TILE_SIZE*TILE_SIZE);
		  compositeTile(x,y, m_pComposeLay, 0);
		  convertTileToPixmap(m_pComposeLay, 0, m_ptiles[y*m_xTiles+x]);
	    }

    //KisTimer::stop("compositeImage");
    emit updated(r);
    
}


/* 
    Renders the part of srcLay which resides in dstTile of dstLay
*/    

void KisImage::renderLayerIntoTile(QRect tileBoundary, const KisLayer *srcLay,
																 KisLayer *dstLay, int dstTile)
{
    int tileNo, tileOffsetX, tileOffsetY, xTile, yTile;

    //puts("renderLayerIntoTile");

    srcLay->findTileNumberAndPos(tileBoundary.topLeft(), &tileNo,
															 &tileOffsetX, &tileOffsetY);
    xTile=tileNo%srcLay->xTiles();
    yTile=tileNo/srcLay->xTiles();
    KIS_DEBUG(render, showi(tileNo); );

    bool renderQ1=true, renderQ2=true, renderQ3=true, renderQ4=true;
    
    if (tileOffsetX<0)
        renderQ1=renderQ3=false;
    if (tileOffsetY<0)
        renderQ2=renderQ1=false;

    KIS_DEBUG(render, showi(tileOffsetX); );
    KIS_DEBUG(render, showi(tileOffsetY); );

    int maxLayerX=TILE_SIZE, maxLayerY=TILE_SIZE;
    
    if (srcLay->boundryTileX(tileNo)) 
    {
        maxLayerX=srcLay->channelLastTileOffsetX();
        if (tileOffsetX>=0)
            renderQ2=false;
        KIS_DEBUG(render, showi(maxLayerX); );
    }
    
    if (tileOffsetX==0)
        renderQ4=false;

    if (srcLay->boundryTileY(tileNo)) 
    {
        maxLayerY=srcLay->channelLastTileOffsetY();
        
	if (tileOffsetY>=0)
	    renderQ3=false;
	KIS_DEBUG(render, showi(maxLayerX); );
    }
    
    if (tileOffsetY==0)
        renderQ4=false;

    KIS_DEBUG(render, showi(renderQ1); );
    KIS_DEBUG(render, showi(renderQ2); );
    KIS_DEBUG(render, showi(renderQ3); );
    KIS_DEBUG(render, showi(renderQ4); );

    // Render quadrants of each tile (either 1, 2 or 4 quadrants get rendered)
    //
    //  ---------
    //  | 1 | 2 |
    //  ---------
    //  | 3 | 4 |
    //  ---------
    //

    KIS_DEBUG(render, {SHOW_POINT(tileBoundary.topLeft());
    printf("tileNo %d, tileOffsetX %d, tileOffsetY %d\n",
	 tileNo, tileOffsetX, tileOffsetY);  });
	
    int renderedToX, renderedToY;

    KIS_DEBUG(render, printf("Test 1: "); );
    
    if (renderQ1) 
    {
        // true => render 1
        renderTileQuadrant(srcLay, tileNo, dstLay, dstTile,
            tileOffsetX, tileOffsetY, 0, 0,
	    TILE_SIZE, TILE_SIZE);
            
        renderedToX=maxLayerX-tileOffsetX;
        renderedToY=maxLayerY-tileOffsetY;
    } 
    else
        KIS_DEBUG(render, puts("ignore"); );
	
    KIS_DEBUG(render, printf("Test 2:"); );
  
    if (renderQ2) 
    {
        // true => render 2
        if (renderQ1)
            renderTileQuadrant(srcLay, tileNo+1, dstLay, dstTile,
			 0, tileOffsetY, maxLayerX-tileOffsetX, 0,
			 TILE_SIZE, TILE_SIZE);
        else
            renderTileQuadrant(srcLay, tileNo, dstLay, dstTile,
			 0, tileOffsetY, -tileOffsetX,0,
			 TILE_SIZE, TILE_SIZE);
    } 
    else
        KIS_DEBUG(render, puts("ignore"));

    KIS_DEBUG(render, printf("Test 3:"); );
  
    if (renderQ3) 
    {
        // true => render 3
        if (renderQ1)
            renderTileQuadrant(srcLay, tileNo+srcLay->xTiles(), dstLay, dstTile,
	        tileOffsetX, 0, 0, maxLayerY-tileOffsetY,
	        TILE_SIZE, TILE_SIZE);
        else
            renderTileQuadrant(srcLay, tileNo, dstLay, dstTile,
		 tileOffsetX, 0, 0, -tileOffsetY,
		 TILE_SIZE, TILE_SIZE);
    } 
    else
        KIS_DEBUG(render, puts("ignore"); );

    KIS_DEBUG(render, printf("Test 4:"); );
    
    // true => render 4
    if (renderQ4) 
    {
        int newTile=tileNo;
        KIS_DEBUG(render, showi(xTile); );
        KIS_DEBUG(render, showi(yTile); );
        if (renderQ1) 
        {
            xTile++; yTile++; newTile+=srcLay->xTiles()+1;
        } 
        else 
        {
            if (renderQ2) { yTile++; newTile+=srcLay->xTiles(); }
	    if (renderQ3) { xTile++; newTile+=1; }
        }
        
        KIS_DEBUG(render, showi(xTile); );
        KIS_DEBUG(render, showi(yTile); );
        
        if ((xTile<srcLay->xTiles()) && (yTile<srcLay->yTiles())) 
        {
            KIS_DEBUG(render, showi(newTile); );
            if (!(renderQ1 && !renderQ2 && !renderQ3)) 
            {
		if (tileOffsetX>0) tileOffsetX=tileOffsetX-TILE_SIZE;
		if (tileOffsetY>0) tileOffsetY=tileOffsetY-TILE_SIZE;
		renderTileQuadrant(srcLay, newTile, dstLay, dstTile,											 0, 0, -tileOffsetX, -tileOffsetY,
		     TILE_SIZE, TILE_SIZE);
            }
        }	
        else
            KIS_DEBUG(render, puts("ignore"); );
    }	
    else
        KIS_DEBUG(render, puts("ignore"); );
}



void KisImage::renderTileQuadrant(const KisLayer *srcLay, int srcTile,
	KisLayer *dstLay, int dstTile,
	int srcX, int srcY, int dstX, int dstY, int w, int h)
{
    for (uchar i = 0; i < srcLay->numChannels(); i++)
	if (srcLay->channelMem(i, srcTile, 0, 0) == 0) return;

    uchar opacity=srcLay->opacity();
	
    // Constrain the width so that the copy is clipped to the overlap
    w = min(min(w, TILE_SIZE-srcX), TILE_SIZE-dstX);
    h = min(min(h, TILE_SIZE-srcY), TILE_SIZE-dstY);
    
    // now constrain if on the boundry of the layer
    if (srcLay->boundryTileX(srcTile))
        w = min(w, srcLay->channelLastTileOffsetX()-srcX);
    if (srcLay->boundryTileY(srcTile))
        h = min(h, srcLay->channelLastTileOffsetY()-srcY);

    // XXX now constrain for the boundry of the Canvas

    int leadIn=(TILE_SIZE-w);

    // FIXME:: Make it work for non-RGB modes
  
    uchar *dptr0 = dstLay->channelMem(0, dstTile, dstX, dstY);
    uchar *dptr1 = dstLay->channelMem(1, dstTile, dstX, dstY);
    uchar *dptr2 = dstLay->channelMem(2, dstTile, dstX, dstY);
    uchar *dptr3 = 0;
    if (m_cMode == cm_RGBA)
	dptr3 = dstLay->channelMem(3, dstTile, dstX, dstY);

    uchar *sptr0 = srcLay->channelMem(0, srcTile, srcX, srcY);
    uchar *sptr1 = srcLay->channelMem(1, srcTile, srcX, srcY);
    uchar *sptr2 = srcLay->channelMem(2, srcTile, srcX, srcY);
    uchar *sptr3 = 0;
    if (m_cMode == cm_RGBA)
	sptr3 = srcLay->channelMem(3, srcTile, srcX, srcY);

    uchar opac,invOpac;
    for(int y = h; y; y--)
    {
        for(int x = w; x; x--)
	{
	    // for prepultiply => invOpac=255-(*alpha*opacity)/255;
		  
	    if (m_cMode == cm_RGBA)
	    {
		  opac = (*sptr3*opacity)/255;
		  invOpac=255-opac;

		  *dptr0++ = (((*dptr0 * *dptr3)/255) * invOpac + *sptr0++ * opac)/255;
		  *dptr1++ = (((*dptr1 * *dptr3)/255) * invOpac + *sptr1++ * opac)/255;
		  *dptr2++ = (((*dptr2 * *dptr3)/255) * invOpac + *sptr2++ * opac)/255;
		  *dptr3++ = *sptr3 + *dptr3 - (*sptr3 * *dptr3)/255;
		  sptr3++;
	    }
            else
	    {
		  invOpac = 255-opacity;
		  *dptr0++ = (*dptr0  * invOpac + *sptr0++ * opacity)/255;
		  *dptr1++ = (*dptr1  * invOpac + *sptr1++ * opacity)/255;
		  *dptr2++ = (*dptr2  * invOpac + *sptr2++ * opacity)/255;
	    }		  
	}
        
	dptr0 += leadIn;
	dptr1 += leadIn;
	dptr2 += leadIn;
	sptr0 += leadIn;
	sptr1 += leadIn;
	sptr2 += leadIn;

	if (m_cMode == cm_RGBA)
	{
	    dptr3 += leadIn;
	    sptr3 += leadIn;
	}
    }
}


KisLayer* KisImage::layerPtr( KisLayer *_layer )
{
    if( _layer == 0 )
        return( m_pCurrentLay );
    return( _layer );
}


void KisImage::setCurrentLayer( int _layer )
{
    m_pCurrentLay = m_layers.at( _layer );
}



void KisImage::convertImageToPixmap(QImage *image, QPixmap *pix)
{
    if (visual==unknown) 
    {
        //TIME_START;
        pix->convertFromImage(*image);
        //TIME_END("convertFromImage");
    } 
    else 
    {
        //TIME_START;
        switch(visual) 
        {
            case rgb565: 
            {
                ushort s;
                ushort *ptr=(ushort *)m_pImgData;
                uchar *qimg=image->bits();
                
                for(int y=0;y<TILE_SIZE;y++)
		    for(int x=0;x<TILE_SIZE;x++) 
                    {
		        s =(*qimg++)>>3;
			s|=(*qimg++ & 252)<<3;
			s|=(*qimg++ & 248)<<8;
			qimg++;
			*ptr++=s;
		    }
            }
                break;

            case rgb888x:
                m_pxi->data=(char*)image->bits();
                break;

            default: break;
        }
        
        XPutImage(pix->x11Display(), pix->handle(), qt_xget_readonly_gc(),
	      m_pxi, 0,0, 0,0, TILE_SIZE, TILE_SIZE);
    
        //TIME_END("fast convertImageToPixmap");
    }
}



void KisImage::convertTileToPixmap(KisLayer *lay, int tileNo, QPixmap *pix)
{
    /*
    Copy the composite image into a QImage so it can be converted to a
    QPixmap.  Note: surprisingly it is not quicker to render directly 
    into a QImage probably due to the CPU cache, it's also useless wrt 
    to other colour  spaces
    */
    
    // FIXME: Make it work for non-RGB images

    uchar *ptr0 = lay->channelMem(2, tileNo, 0, 0);
    uchar *ptr1 = lay->channelMem(1, tileNo, 0, 0);
    uchar *ptr2 = lay->channelMem(0, tileNo, 0, 0);

    for(int y = 0; y < TILE_SIZE; y++)
    {
        uchar *ptr = m_img.scanLine(y);
	for(int x = TILE_SIZE; x; x--)
	{
	    *ptr++ = *ptr0++;
	    *ptr++ = *ptr1++;
	    *ptr++ = *ptr2++;
	    ptr++;
	}
    }

    // Construct the relevant pixmap
    convertImageToPixmap(&m_img, pix);
}


void KisImage::mergeAllLayers()
{
    QList<KisLayer> l;
    KisLayer *lay = m_layers.first();

    while(lay)
    {
        l.append(lay);
        lay = m_layers.next();
    }
    
    mergeLayers(l);
}


void KisImage::mergeVisibleLayers()
{
    QList<KisLayer> l;

    KisLayer *lay = m_layers.first();

    while(lay)
    {
        if(lay->visible())
	    l.append(lay);
        lay = m_layers.next();
    }
    
    mergeLayers(l);
}


void KisImage::mergeLinkedLayers()
{
    QList<KisLayer> l;

    KisLayer *lay = m_layers.first();

    while(lay)
    {
        if (lay->linked())
	    l.append(lay);
        lay = m_layers.next();
    }
    
    mergeLayers(l);
}



void KisImage::mergeLayers(QList<KisLayer> list)
{
    list.setAutoDelete(false);

    KisLayer *a, *b;
    QRect newRect;

    a = list.first();
    while(a)
    {
        newRect.unite(a->imageExtents());
        //a->renderOpacityToAlpha();
        a = list.next();
    }

    while((a = list.first()) && (b = list.next()))
    {
        if (!a || !b) break;
        QRect ar = a->imageExtents();
        QRect br = b->imageExtents();
      
        QRect urect = ar.unite(br);
      
        // allocate out tiles if required
        a->allocateRect(urect);
        b->allocateRect(urect);
      
        // rect in layer coords (offset from tileExtents.topLeft())
        QRect rect = urect;
        rect.moveTopLeft(urect.topLeft() - a->tileExtents().topLeft());
      
        // workout which tiles in the layer need to be updated
        int minYTile=rect.top() / TILE_SIZE;
        int maxYTile=rect.bottom() / TILE_SIZE;
        int minXTile=rect.left() / TILE_SIZE;
        int maxXTile=rect.right() / TILE_SIZE;
      
        QRect tileBoundary;
  
        for(int y=minYTile; y<=maxYTile; y++)
	{
	    for(int x=minXTile; x<=maxXTile; x++)
	    {
	        int dstTile = y * a->xTiles() + x;
	        tileBoundary = a->tileRect(dstTile);
	        renderLayerIntoTile(tileBoundary, b, a, dstTile);
	    }
	}
      
        list.remove(b);
        m_layers.remove(b);
        
        if( m_pCurrentLay == b )
	{
	    if(m_layers.count() != 0)
	        m_pCurrentLay = m_layers.at(0);
	    else
	        m_pCurrentLay = NULL;
	}
        
        delete b;
    }
    
    emit layersUpdated();
    compositeImage(newRect);
}


void KisImage::upperLayer( unsigned int _layer )
{
    ASSERT( _layer < m_layers.count() );

    if( _layer > 0 )
    {
        KisLayer *pLayer = m_layers.take( _layer );
        m_layers.insert( _layer - 1, pLayer );
    }
}


void KisImage::lowerLayer( unsigned int _layer )
{
    ASSERT( _layer < m_layers.count() );

    if( _layer < ( m_layers.count() - 1 ) )
    {
        KisLayer *pLayer = m_layers.take( _layer );
        m_layers.insert( _layer + 1, pLayer );
    }
}


void KisImage::setFrontLayer( unsigned int _layer )
{
    ASSERT( _layer < m_layers.count() );

    if( _layer < ( m_layers.count() - 1 ) )
    {
        KisLayer *pLayer = m_layers.take( _layer );
        m_layers.append( pLayer );
    }
}


void KisImage::setBackgroundLayer( unsigned int _layer )
{
    ASSERT( _layer < m_layers.count() );

    if( _layer > 0 )
    {
        KisLayer *pLayer = m_layers.take( _layer );
        m_layers.insert( 0, pLayer );
    }
}

#include "kis_image.moc"
