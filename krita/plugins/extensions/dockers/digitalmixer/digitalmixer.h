/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _DIGITALMIXER_H_
#define _DIGITALMIXER_H_

#include <kparts/plugin.h>

class KisView2;

/**
 * Template of view plugin
 */
class DigitalMixerPlugin : public KParts::Plugin
{
    Q_OBJECT
    public:
        DigitalMixerPlugin(QObject *parent, const QStringList &);
        virtual ~DigitalMixerPlugin();
    private:
        KisView2* m_view;
};

#endif
