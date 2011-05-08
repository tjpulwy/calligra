/*
 * This file is part of the KDE project
 *
 * Copyright (C) 2011 Shantanu Tushar <jhahoneyk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#ifndef PAVIEW_H
#define PAVIEW_H

#include <libs/kopageapp/KoPAViewBase.h>

class KPrDocument;
class KoPACanvasBase;

class PAView : public KoPAViewBase
{

public:
    PAView(KoPACanvasBase *canvas, KPrDocument *prDocument, KoZoomController *zoomController,
        KoZoomHandler *zoomHandler);
    virtual ~PAView();
    virtual void setShowRulers(bool show);
    virtual void editPaste();
    virtual void pagePaste();
    virtual void insertPage();
    virtual void updatePageNavigationActions();
    virtual void setActionEnabled(int actions, bool enable);
    virtual void navigatePage(KoPageApp::PageNavigation pageNavigation);
    virtual KoPAPageBase* activePage() const;
    virtual void setActivePage(KoPAPageBase* page);
    virtual void doUpdateActivePage(KoPAPageBase* page);
    virtual KoZoomController* zoomController() const;
    virtual KoPADocument* kopaDocument() const;
    virtual KoPACanvasBase* kopaCanvas() const;

private:
    KoPACanvasBase *m_paCanvas;
    KPrDocument *m_prDocument;
    KoZoomController *m_zoomController;
    KoZoomHandler *m_zoomHandler;
    KoPAPageBase *m_page;
};

#endif // PAVIEW_H
