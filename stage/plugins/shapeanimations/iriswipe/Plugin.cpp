/* This file is part of the KDE project
 * Copyright (C) 2008 Carlos Licea <carlos.licea@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Plugin.h"
#include <kpluginfactory.h>
#include <KPrTransitionFilterRegistry.h>

//#include "irisWipe/KPrIrisWipeEffectFactory.h"
//#include "triangleWipe/KPrTriangleWipeEffectFactory.h"
//#include "arrowHeadWipe/KPrArrowHeadWipeEffectFactory.h"
#include "ellipseWipe/KPrEllipseWipeFilterEffectFactory.h"
//#include "roundRectWipe/KPrRoundRectWipeEffectFactory.h"
//#include "pentagonWipe/KPrPentagonWipeEffectFactory.h"
//#include "hexagonWipe/KPrHexagonWipeEffectFactory.h"
//#include "starWipe/KPrStarWipeEffectFactory.h"
//#include "eyeWipe/KPrEyeWipeEffectFactory.h"
//#include "miscShapeWipe/KPrMiscShapeWipeEffectFactory.h"


K_PLUGIN_FACTORY(PluginFactory, registerPlugin<Plugin>();)
K_EXPORT_PLUGIN(PluginFactory("KPrTransitionFilterEffect"))

Plugin::Plugin(QObject *parent, const QVariantList &)
    : QObject(parent)
{
   // KPrPageEffectRegistry::instance()->add(new KPrIrisWipeEffectFactory());
    //KPrPageEffectRegistry::instance()->add(new KPrTriangleWipeEffectFactory());
    //KPrPageEffectRegistry::instance()->add(new KPrArrowHeadWipeEffectFactory());
    KPrTransitionFilterRegistry::instance()->add(new KPrEllipseWipeFilterEffectFactory());
    //KPrPageEffectRegistry::instance()->add(new KPrRoundRectWipeEffectFactory());
    //KPrPageEffectRegistry::instance()->add(new KPrPentagonWipeEffectFactory());
    //KPrPageEffectRegistry::instance()->add(new KPrHexagonWipeEffectFactory());
    //KPrPageEffectRegistry::instance()->add(new KPrStarWipeEffectFactory());
    //KPrPageEffectRegistry::instance()->add(new KPrEyeWipeEffectFactory());
    //KPrPageEffectRegistry::instance()->add(new KPrMiscShapeWipeEffectFactory());
}

#include "Plugin.moc"
