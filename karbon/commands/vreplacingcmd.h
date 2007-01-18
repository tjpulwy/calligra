/* This file is part of the KDE project
   Copyright (C) 2002, The Karbon Developers

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef __VREPLACINGCMD_H__
#define __VREPLACINGCMD_H__


#include "vcommand.h"
#include <karbon_export.h>
class QString;
class VSelection;


/**
 * VReplacingCmd is a generic command. Derive from it if you plan to do complex
 * transformations upon selected objects which make it necessary to replace
 * each object as a whole with a new object.
 */

class KARBONCOMMAND_EXPORT VReplacingCmd : public VCommand
{
public:
	virtual void redo();
	virtual void undo();

protected:
	/**
	 * Make it "abstract".
	 */
	VReplacingCmd( VDocument* doc, const QString& name );
	virtual ~VReplacingCmd();

private:
	VSelection* m_oldObjects;
	VSelection* m_newObjects;
};

#endif

