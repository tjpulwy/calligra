/* This file is part of the KDE project
   Copyright (C) 2003 Jaroslaw Staniek <js@iidea.pl>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <kexidb/connectiondata.h>

#include <kexidb/drivermanager.h>

#include <qfileinfo.h>
#include <qdir.h>

#include <klocale.h>

using namespace KexiDB;

namespace KexiDB {
//! @internal
class ConnectionData::Private {
public:
	Private() { 
		dummy=false;
	}
	~Private() {}
	bool dummy;
};
}

/*================================================================*/

ConnectionDataBase::ConnectionDataBase()
	: id(-1), port(0), useLocalSocketFile(false), savePassword(false)
{
}

/*================================================================*/

ConnectionData::ConnectionData()
: QObject()
, ConnectionDataBase()
, priv(new ConnectionData::Private())
{
}

ConnectionData::ConnectionData(const ConnectionData& cd)
: QObject()
, ConnectionDataBase()
, priv(0)
{
	static_cast<ConnectionData&>(*this) = static_cast<const ConnectionData&>(cd);//copy data members
/*	if (&cd != this) {
		static_cast<ConnectionDataBase&>(*this) = static_cast<const ConnectionDataBase&>(cd);//copy data members
	}
	priv = new ConnectionData::Private();*/
//todo: copy priv contents if not empty	*d = *cd.d;
}

ConnectionData::~ConnectionData()
{
	delete priv;
	priv = 0;
}

ConnectionData& ConnectionData::operator=(const ConnectionData& cd)
{
	delete priv; //this is old
	static_cast<ConnectionDataBase&>(*this) = static_cast<const ConnectionDataBase&>(cd);//copy data members
	priv = new ConnectionData::Private();
	*priv = *cd.priv;
	return *this;
}

void ConnectionData::setFileName( const QString& fn )
{
	QFileInfo file(fn);
	if (!fn.isEmpty() && m_fileName != file.absFilePath()) {
		m_fileName = QDir::convertSeparators(file.absFilePath());
		m_dbPath = QDir::convertSeparators(file.dirPath(true));
		m_dbFileName = file.fileName();
	}
}

QString ConnectionData::serverInfoString(bool addUser) const
{
	const QString& i18nFile = i18n("file");
	
	if (!m_dbFileName.isEmpty())
		return i18nFile+": "+(m_dbPath.isEmpty() ? "" : m_dbPath 
			+ QDir::separator()) + m_dbFileName;
	
	DriverManager man;
	if (!driverName.isEmpty()) {
		Driver::Info info = man.driverInfo(driverName);
		if (!info.name.isEmpty() && info.fileBased)
			return QString("<")+i18nFile+">";
	}
	
	return ( (userName.isEmpty() || !addUser) ? QString("") : (userName+"@"))
		+ (hostName.isEmpty() ? QString("localhost") : hostName)
		+ (port!=0 ? (QString(":")+QString::number(port)) : QString::null);
}

