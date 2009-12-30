/*
 *  Copyright (c) 2008,2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_spray_paintop_settings_widget.h"

#include "kis_sprayop_option.h"
#include "kis_spray_paintop_settings.h"
#include "kis_spray_shape_option.h"

#include <kis_color_option.h>
#include <kis_paintop_options_widget.h>
#include <kis_paint_action_type_option.h>

#include <kis_pressure_rotation_option.h>
#include <kis_pressure_opacity_option.h>
#include <kis_pressure_size_option.h>

KisSprayPaintOpSettingsWidget:: KisSprayPaintOpSettingsWidget(QWidget* parent)
        : KisPaintOpOptionsWidget(parent)
{
    m_sprayOption =  new KisSprayOpOption();
    m_sprayShapeOption = new KisSprayShapeOption();
    m_ColorOption = new KisColorOption();
    m_paintActionTypeOption = new KisPaintActionTypeOption();
    m_rotationOption = new KisPressureRotationOption();
    m_sizeOption = new KisPressureSizeOption();
    m_opacityOption = new KisPressureOpacityOption();
    
    addPaintOpOption(m_sprayOption);
    addPaintOpOption(m_sprayShapeOption);
//     addPaintOpOption(m_ColorOption);
//     addPaintOpOption(m_opacityOption);
//     addPaintOpOption(m_sizeOption);
//     addPaintOpOption(m_rotationOption);
    addPaintOpOption(m_paintActionTypeOption);
}

KisSprayPaintOpSettingsWidget::~ KisSprayPaintOpSettingsWidget()
{
    delete m_sprayOption;
    delete m_sprayShapeOption;
    delete m_ColorOption;
    delete m_paintActionTypeOption;
    delete m_rotationOption;
    delete m_sizeOption;
    delete m_opacityOption;

}

KisPropertiesConfiguration*  KisSprayPaintOpSettingsWidget::configuration() const
{
    KisSprayPaintOpSettings* config = new KisSprayPaintOpSettings();
    config->setOptionsWidget(const_cast<KisSprayPaintOpSettingsWidget*>(this));
    config->setProperty("paintop", "spraybrush"); // XXX: make this a const id string
    writeConfiguration(config);
    return config;
}
