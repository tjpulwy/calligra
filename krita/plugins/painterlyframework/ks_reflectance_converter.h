/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#ifndef KS_REF_CONVERTER_H_
#define KS_REF_CONVERTER_H_

class KSReflectanceConverter {

    public:
        KSReflectanceConverter() {}
        KSReflectanceConverter(float whiteS, float blackK);
        ~KSReflectanceConverter();

        void KS2Reflectance(float K, float S, float &R) const;
        void Reflectance2KS(float R, float &K, float &S) const;

    private:
        float S_w, K_b;

};

#endif // KS_REF_CONVERTER_H_
