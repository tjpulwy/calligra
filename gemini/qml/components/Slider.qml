/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

import QtQuick 2.0

Item {
    id: base;
    height: Constants.LargeFontSize;
    property double value: 0;
    property double exponentialValue: 0;
    property bool highPrecision: false;
    property bool useExponentialValue: false;
    onValueChanged: handle.resetHandle();
    onExponentialValueChanged: handle.resetHandle(true);

    property alias border: fill.border;
    property alias background: fill.color;

    Rectangle {
        id: fill;
        anchors {
            //fill: parent;
            right: parent.right;
            left: parent.left;
            verticalCenter: parent.verticalCenter;
        }
        height: 6;
        color: Settings.theme.color("components/slider/background/fill");
        radius: 3;
        smooth: true;
        border.width: 1;
        border.color: "#CCCCCC";
        opacity: 0.6;
    }
    MouseArea {
        anchors.fill: parent;
        function handlePos(mouse) {
            var position = mouse.x - (handle.height / 2);
            handle.x = position < 0 ? 0 : position > base.width - handle.width ? base.width - handle.width : position;
            handle.resetHandle(useExponentialValue);
        }
        onClicked: handlePos(mouse);
        onPositionChanged: handlePos(mouse);
    }
    Rectangle {
        id: handle;
        property bool settingSelf: false;
        property bool settingExp: false;
        function resetHandle(resetExponential)
        {
            if (!settingSelf) {
                // Yes, this seems very odd. However, one cannot assign something undefined to a bool property, so...
                settingExp = resetExponential ? true : false;
                // This is required as width is not set if we are not done initialising yet.
                if (base.width === 0)
                    return;
                var newX = 0;
                if (settingExp) {
                    var newX = Math.round(Math.log(1 + (base.exponentialValue / 100 * 15)) / 2.77258872 * (handle.maximumX - handle.minimumX) + handle.minimumX);
                }
                else {
                    var newX = Math.round(base.value / 100 * (handle.maximumX - handle.minimumX) + handle.minimumX);
                }
                if (newX !== handle.x)
                    handle.x = newX;
                settingExp = false;
            }
        }
        y: 0
        x: 2
        function calculateWidth(x)
        {
            var v = 100 * ((Math.exp(2.77258872 * (x / 100)) - 1) / 15);

            // Check boundary conditions as Math.exp may round off too much.
            if (x === 0 || x === 100) {
                return x;
            }

            return Math.min(100, Math.max(0, v));
        }
        onXChanged: {
            if (settingExp || settingSelf)
                 return;
            settingSelf = true;
            if (highPrecision) {
                var newValue = ((handle.x - handle.minimumX) * 100) / (handle.maximumX - handle.minimumX);
                if (base.value != newValue) {
                    base.exponentialValue = calculateWidth(newValue);
                    base.value = Math.max(0, newValue);
                }
            }
            else {
                var newValue = Math.round( ((handle.x - handle.minimumX) * 100) / (handle.maximumX - handle.minimumX) );
                if (base.value != newValue) {
                    base.exponentialValue = calculateWidth(newValue);
                    base.value = Math.max(0, newValue);
                }
            }
            settingSelf = false;
        }
        height: parent.height;
        width: height;
        //radius: (height / 2) + 1;
        //color: Settings.theme.color("components/slider/handle/fill");
        //border.width: 1;
        //border.color: Settings.theme.color("components/slider/handle/border");
        //smooth: true;
        color: "transparent";
        property int minimumX: 2;
        property int maximumX: base.width - handle.width - 2;
        onMaximumXChanged: handle.resetHandle(base.useExponentialValue);
        Image {
            anchors.fill: parent;
            source: Settings.theme.icon("SVG-IMGTOOLS-SliderHandle-1");
            sourceSize.width: width > height ? height : width;
            sourceSize.height: width > height ? height : width;
        }
    }
}
