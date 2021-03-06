/* This file is part of the KDE project
   Copyright (C) 2007, 2011, 2012 Dag Andersen danders@get2net>

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

#include "kptresourceappointmentsmodel.h"

#include "kptglobal.h"
#include "kptcommonstrings.h"
#include "kptappointment.h"
#include "kptcommand.h"
#include "kpteffortcostmap.h"
#include "kptitemmodelbase.h"
#include "kptcalendar.h"
#include "kptduration.h"
#include "kptnode.h"
#include "kptproject.h"
#include "kpttask.h"
#include "kptresource.h"
#include "kptdatetime.h"
#include "kptdebug.h"

#include <KFormat>

#include <QDate>
#include <QList>
#include <QLocale>

#include <KGanttGlobal>


namespace KPlato
{

ResourceAppointmentsItemModel::ResourceAppointmentsItemModel( QObject *parent )
    : ItemModelBase( parent ),
    m_group( 0 ),
    m_resource( 0 ),
    m_showInternal( true ),
    m_showExternal( true )
{
}

ResourceAppointmentsItemModel::~ResourceAppointmentsItemModel()
{
}

void ResourceAppointmentsItemModel::slotResourceToBeInserted( const ResourceGroup *group, int row )
{
    debugPlan<<group->name()<<row;
    Q_ASSERT( m_group == 0 );
    m_group = const_cast<ResourceGroup*>(group);
    QModelIndex i = index( group );
    beginInsertRows( i, row, row );
}

void ResourceAppointmentsItemModel::slotResourceInserted( const Resource *r )
{
    debugPlan<<r->name();
    Q_ASSERT( r->parentGroup() == m_group );
    endInsertRows();
    m_group = 0;
    refresh();
    connect( r, SIGNAL(externalAppointmentToBeAdded(Resource*,int)), this, SLOT(slotAppointmentToBeInserted(Resource*,int)) );
    connect( r, SIGNAL(externalAppointmentAdded(Resource*,Appointment*)), this, SLOT(slotAppointmentInserted(Resource*,Appointment*)) );
    connect( r, SIGNAL(externalAppointmentToBeRemoved(Resource*,int)), this, SLOT(slotAppointmentToBeRemoved(Resource*,int)) );
    connect( r, SIGNAL(externalAppointmentRemoved()), this, SLOT(slotAppointmentRemoved()) );
    connect( r, SIGNAL(externalAppointmentChanged(Resource*,Appointment*)), this, SLOT(slotAppointmentChanged(Resource*,Appointment*)) );
}

void ResourceAppointmentsItemModel::slotResourceToBeRemoved( const Resource *r )
{
    debugPlan<<r->name();
    int row = r->parentGroup()->indexOf( r );
    beginRemoveRows( index( r->parentGroup() ), row, row );
    disconnect( r, SIGNAL(externalAppointmentToBeAdded(Resource*,int)), this, SLOT(slotAppointmentToBeInserted(Resource*,int)) );
    disconnect( r, SIGNAL(externalAppointmentAdded(Resource*,Appointment*)), this, SLOT(slotAppointmentInserted(Resource*,Appointment*)) );
    disconnect( r, SIGNAL(externalAppointmentToBeRemoved(Resource*,int)), this, SLOT(slotAppointmentToBeRemoved(Resource*,int)) );
    disconnect( r, SIGNAL(externalAppointmentRemoved()), this, SLOT(slotAppointmentRemoved()) );
    disconnect( r, SIGNAL(externalAppointmentChanged(Resource*,Appointment*)), this, SLOT(slotAppointmentChanged(Resource*,Appointment*)) );

}

void ResourceAppointmentsItemModel::slotResourceRemoved( const Resource *resource )
{
    Q_UNUSED(resource);
    //debugPlan<<resource->name();
    endRemoveRows();
    refresh();
}

void ResourceAppointmentsItemModel::slotResourceGroupToBeInserted( const ResourceGroup *group, int row )
{
    //debugPlan<<group->name()<<endl;
    Q_ASSERT( m_group == 0 );
    m_group = const_cast<ResourceGroup*>(group);
    beginInsertRows( QModelIndex(), row, row );
}

void ResourceAppointmentsItemModel::slotResourceGroupInserted( const ResourceGroup *group )
{
    //debugPlan<<group->name()<<endl;
    Q_ASSERT( group == m_group ); Q_UNUSED( group );
    endInsertRows();
    m_group = 0;
}

void ResourceAppointmentsItemModel::slotResourceGroupToBeRemoved( const ResourceGroup *group )
{
    //debugPlan<<group->name()<<endl;
    Q_ASSERT( m_group == 0 );
    m_group = const_cast<ResourceGroup*>(group);
    int row = index( group ).row();
    beginRemoveRows( QModelIndex(), row, row );
}

void ResourceAppointmentsItemModel::slotResourceGroupRemoved( const ResourceGroup *group )
{
    //debugPlan<<group->name()<<endl;
    Q_ASSERT( group == m_group ); Q_UNUSED( group );
    endRemoveRows();
    m_group = 0;
}

void ResourceAppointmentsItemModel::slotAppointmentToBeInserted( Resource *r, int row )
{
    Q_UNUSED(r);
    Q_UNUSED(row);
}

void ResourceAppointmentsItemModel::slotAppointmentInserted( Resource *r, Appointment *a )
{
    Q_UNUSED(r);
    Q_UNUSED(a);
    refreshData();
    reset();
}

void ResourceAppointmentsItemModel::slotAppointmentToBeRemoved( Resource *r, int row )
{
    Q_UNUSED(r);
    Q_UNUSED(row);
}

void ResourceAppointmentsItemModel::slotAppointmentRemoved()
{
    refreshData();
    reset();
}

void ResourceAppointmentsItemModel::slotAppointmentChanged( Resource *r, Appointment *a )
{
    int row = rowNumber( r, a );
    Q_ASSERT( row >= 0 );
    refreshData();
    emit dataChanged( createExternalAppointmentIndex( row, 0, a ), createExternalAppointmentIndex( row, columnCount() - 1, a ) );
}

void ResourceAppointmentsItemModel::slotProjectCalculated( ScheduleManager *sm )
{
    if ( sm == m_manager ) {
        setScheduleManager( sm );
    }
}

int ResourceAppointmentsItemModel::rowNumber( Resource *res, Appointment *a ) const
{
    int r = 0;
    if ( m_showInternal ) {
        r = res->appointments( id() ).indexOf( a );
        if ( r > -1 ) {
            return r;
        }
        r = res->numAppointments();
    }
    if ( m_showExternal ) {
        int rr = res->externalAppointmentList().indexOf( a );
        if ( rr > -1 ) {
            return r + rr;
        }
    }
    return -1;
}

void ResourceAppointmentsItemModel::setShowInternalAppointments( bool show )
{
    if ( m_showInternal == show ) {
        return;
    }
    m_showInternal = show;
    refreshData();
    reset();
}

void ResourceAppointmentsItemModel::setShowExternalAppointments( bool show )
{
    if ( m_showExternal == show ) {
        return;
    }
    m_showExternal = show;
    refreshData();
    reset();
}

void ResourceAppointmentsItemModel::setProject( Project *project )
{
    debugPlan;
    if ( m_project ) {
        disconnect(m_project, SIGNAL(aboutToBeDeleted()), this, SLOT(projectDeleted()));

        disconnect( m_project, SIGNAL(resourceChanged(Resource*)), this, SLOT(slotResourceChanged(Resource*)) );

        disconnect( m_project, SIGNAL(resourceGroupChanged(ResourceGroup*)), this, SLOT(slotResourceGroupChanged(ResourceGroup*)) );

        disconnect( m_project, SIGNAL(resourceGroupToBeAdded(const ResourceGroup*,int)), this, SLOT(slotResourceGroupToBeInserted(const ResourceGroup*,int)) );

        disconnect( m_project, SIGNAL(resourceGroupToBeRemoved(const ResourceGroup*)), this, SLOT(slotResourceGroupToBeRemoved(const ResourceGroup*)) );

        disconnect( m_project, SIGNAL(resourceToBeAdded(const ResourceGroup*,int)), this, SLOT(slotResourceToBeInserted(const ResourceGroup*,int)) );

        disconnect( m_project, SIGNAL(resourceToBeRemoved(const Resource*)), this, SLOT(slotResourceToBeRemoved(const Resource*)) );

        disconnect( m_project, SIGNAL(resourceGroupAdded(const ResourceGroup*)), this, SLOT(slotResourceGroupInserted(const ResourceGroup*)) );

        disconnect( m_project, SIGNAL(resourceGroupRemoved(const ResourceGroup*)), this, SLOT(slotResourceGroupRemoved(const ResourceGroup*)) );

        disconnect( m_project, SIGNAL(resourceAdded(const Resource*)), this, SLOT(slotResourceInserted(const Resource*)) );

        disconnect( m_project, SIGNAL(resourceRemoved(const Resource*)), this, SLOT(slotResourceRemoved(const Resource*)) );

        disconnect( m_project, SIGNAL(defaultCalendarChanged(Calendar*)), this, SLOT(slotCalendarChanged(Calendar*)) );

        disconnect( m_project, SIGNAL(projectCalculated(ScheduleManager*)), this, SLOT(slotProjectCalculated(ScheduleManager*)) );

        disconnect( m_project, SIGNAL(scheduleManagerChanged(ScheduleManager*)), this, SLOT(slotProjectCalculated(ScheduleManager*)) );

        foreach ( Resource *r, m_project->resourceList() ) {
            disconnect( r, SIGNAL(externalAppointmentToBeAdded(Resource*,int)), this, SLOT(slotAppointmentToBeInserted(Resource*,int)) );
            disconnect( r, SIGNAL(externalAppointmentAdded(Resource*,Appointment*)), this, SLOT(slotAppointmentInserted(Resource*,Appointment*)) );
            disconnect( r, SIGNAL(externalAppointmentToBeRemoved(Resource*,int)), this, SLOT(slotAppointmentToBeRemoved(Resource*,int)) );
            disconnect( r, SIGNAL(externalAppointmentRemoved()), this, SLOT(slotAppointmentRemoved()) );
            disconnect( r, SIGNAL(externalAppointmentChanged(Resource*,Appointment*)), this, SLOT(slotAppointmentChanged(Resource*,Appointment*)) );
        }
    }
    m_project = project;
    if ( m_project ) {
        connect(m_project, SIGNAL(aboutToBeDeleted()), this, SLOT(projectDeleted()));
        connect( m_project, SIGNAL(resourceChanged(Resource*)), this, SLOT(slotResourceChanged(Resource*)) );
        connect( m_project, SIGNAL(resourceGroupChanged(ResourceGroup*)), this, SLOT(slotResourceGroupChanged(ResourceGroup*)) );

        connect( m_project, SIGNAL(resourceGroupToBeAdded(const ResourceGroup*,int)), this, SLOT(slotResourceGroupToBeInserted(const ResourceGroup*,int)) );

        connect( m_project, SIGNAL(resourceGroupToBeRemoved(const ResourceGroup*)), this, SLOT(slotResourceGroupToBeRemoved(const ResourceGroup*)) );

        connect( m_project, SIGNAL(resourceToBeAdded(const ResourceGroup*,int)), this, SLOT(slotResourceToBeInserted(const ResourceGroup*,int)) );

        connect( m_project, SIGNAL(resourceToBeRemoved(const Resource*)), this, SLOT(slotResourceToBeRemoved(const Resource*)) );

        connect( m_project, SIGNAL(resourceGroupAdded(const ResourceGroup*)), this, SLOT(slotResourceGroupInserted(const ResourceGroup*)) );

        connect( m_project, SIGNAL(resourceGroupRemoved(const ResourceGroup*)), this, SLOT(slotResourceGroupRemoved(const ResourceGroup*)) );

        connect( m_project, SIGNAL(resourceAdded(const Resource*)), this, SLOT(slotResourceInserted(const Resource*)) );

        connect( m_project, SIGNAL(resourceRemoved(const Resource*)), this, SLOT(slotResourceRemoved(const Resource*)) );

        connect( m_project, SIGNAL(defaultCalendarChanged(Calendar*)), this, SLOT(slotCalendarChanged(Calendar*)) );

        connect( m_project, SIGNAL(projectCalculated(ScheduleManager*)), this, SLOT(slotProjectCalculated(ScheduleManager*)) );

        connect( m_project, SIGNAL(scheduleManagerChanged(ScheduleManager*)), this, SLOT(slotProjectCalculated(ScheduleManager*)) );

        foreach ( Resource *r, m_project->resourceList() ) {
            connect( r, SIGNAL(externalAppointmentToBeAdded(Resource*,int)), this, SLOT(slotAppointmentToBeInserted(Resource*,int)) );
            connect( r, SIGNAL(externalAppointmentAdded(Resource*,Appointment*)), this, SLOT(slotAppointmentInserted(Resource*,Appointment*)) );
            connect( r, SIGNAL(externalAppointmentToBeRemoved(Resource*,int)), this, SLOT(slotAppointmentToBeRemoved(Resource*,int)) );
            connect( r, SIGNAL(externalAppointmentRemoved()), this, SLOT(slotAppointmentRemoved()) );
            connect( r, SIGNAL(externalAppointmentChanged(Resource*,Appointment*)), this, SLOT(slotAppointmentChanged(Resource*,Appointment*)) );
        }
    }
    refreshData();
    reset();
    emit refreshed();
}

QDate ResourceAppointmentsItemModel::startDate() const
{
    if ( m_project && m_manager ) {
        return m_project->startTime( id() ).date();
    }
    return QDate::currentDate();
}

QDate ResourceAppointmentsItemModel::endDate() const
{
    if ( m_project && m_manager ) {
        return m_project->endTime( id() ).date();
    }
    return QDate::currentDate();
}

void ResourceAppointmentsItemModel::setScheduleManager( ScheduleManager *sm )
{
    debugPlan<<sm;
    m_manager = sm;
    refreshData();
    reset();
    emit refreshed();
}

long ResourceAppointmentsItemModel::id() const
{
    return m_manager == 0 ? -1 : m_manager->scheduleId();
}

Qt::ItemFlags ResourceAppointmentsItemModel::flags( const QModelIndex &index ) const
{
    Qt::ItemFlags flags = ItemModelBase::flags( index );
    return flags &= ~Qt::ItemIsEditable;
}


QModelIndex ResourceAppointmentsItemModel::parent( const QModelIndex &idx ) const
{
    if ( !idx.isValid() || m_project == 0 || m_manager == 0 ) {
        warnPlan<<"No data "<<idx;
        return QModelIndex();
    }

    QModelIndex p;
    if ( ! p.isValid() ) {
        Resource *r = resource( idx );
        if ( r ) {
            int row = m_project->indexOf( r->parentGroup() );
            p = createGroupIndex( row, 0, r->parentGroup() );
            //debugPlan<<"Parent:"<<p<<r->parentGroup()->name();
            Q_ASSERT( p.isValid() );
        }
    }
    if ( ! p.isValid() && m_showInternal ) {
        Appointment *a = appointment( idx );
        if ( a && a->resource() && a->resource()->resource() ) {
            Resource *r = a->resource()->resource();
            int row = r->parentGroup()->indexOf( r );
            p = createResourceIndex( row, 0, r );
            //debugPlan<<"Parent:"<<p<<r->name();
            Q_ASSERT( p.isValid() );
        }
    }
    if ( ! p.isValid() && m_showExternal ) {
        Appointment *a = externalAppointment( idx );
        Resource *r = parent( a );
        if ( r ) {
            int row = r->parentGroup()->indexOf( r );
            p = createResourceIndex( row, 0, r );
        }
    }
    if ( ! p.isValid() ) {
        //debugPlan<<"Parent:"<<p;
    }
    //debugPlan<<"Child :"<<idx;
    return p;
}

Resource *ResourceAppointmentsItemModel::parent( const Appointment *a ) const
{
    if ( a == 0 || m_project == 0 ) {
        return 0;
    }
    foreach ( Resource *r, m_project->resourceList() ) {
        if ( r->appointments( id() ).contains( const_cast<Appointment*>( a ) ) ) {
            return r;
        }
        if ( r->externalAppointmentList().contains( const_cast<Appointment*>( a ) ) ) {
            return r;
        }
    }
    return 0;
}

QModelIndex ResourceAppointmentsItemModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( m_project == 0 || m_manager == 0 ) {
        return QModelIndex();
    }
    if ( ! parent.isValid() ) {
        if ( row < m_project->numResourceGroups() ) {
            //debugPlan<<"Group: "<<m_project->resourceGroupAt( row )<<endl;
            return createGroupIndex( row, column, m_project->resourceGroupAt( row ) );
        }
        return QModelIndex();
    }
    ResourceGroup *g = resourcegroup( parent );
    if ( g ) {
        if ( row < g->numResources() ) {
            //debugPlan<<"Resource: "<<g->resourceAt( row )<<endl;
            return createResourceIndex( row, column, g->resourceAt( row ) );
        }
        return QModelIndex();
    }
    Resource *r = resource( parent );
    if ( r && ( m_showInternal || m_showExternal ) ) {
        int num = m_showInternal ? r->numAppointments( id() ) : 0;
        if ( row < num ) {
            //debugPlan<<"Appointment: "<<r->appointmentAt( row, m_manager->scheduleId() );
            return createAppointmentIndex( row, column, r->appointmentAt( row, id() ) );
        }
        int extRow = row - num;
        //debugPlan<<"Appointment: "<<r->externalAppointmentList().value( extRow );
        Q_ASSERT( extRow >= 0 && extRow < r->externalAppointmentList().count() );
        return createExternalAppointmentIndex( row, column, r->externalAppointmentList().value( extRow ) );
    }
    return QModelIndex();
}

QModelIndex ResourceAppointmentsItemModel::index( const Resource *resource ) const
{
    if ( m_project == 0 || resource == 0 ) {
        return QModelIndex();
    }
    Resource *r = const_cast<Resource*>(resource);
    int row = -1;
    ResourceGroup *par = r->parentGroup();
    if ( par ) {
        row = par->indexOf( r );
        return createResourceIndex( row, 0, r );
    }
    return QModelIndex();
}

QModelIndex ResourceAppointmentsItemModel::index( const ResourceGroup *group ) const
{
    if ( m_project == 0 || group == 0 ) {
        return QModelIndex();
    }
    ResourceGroup *g = const_cast<ResourceGroup*>(group);
    int row = m_project->indexOf( g );
    return createGroupIndex( row, 0, g );

}

void ResourceAppointmentsItemModel::refresh()
{
    refreshData();
    emit refreshed();
}

void ResourceAppointmentsItemModel::refreshData()
{
    long id = m_manager == 0 ? -1 : m_manager->scheduleId();
    //debugPlan<<"Schedule id: "<<id<<endl;
    QDate start;
    QDate end;
    QMap<const Appointment*, EffortCostMap> ec;
    QMap<const Appointment*, EffortCostMap> extEff;
    foreach ( Resource *r, m_project->resourceList() ) {
        foreach (Appointment* a, r->appointments( id )) {
            QDate s = a->startTime().date();
            QDate e = a->endTime().date();
            ec[ a ] = a->plannedPrDay( s, e );
            if ( ! start.isValid() || s < start ) {
                start = s;
            }
            if ( ! end.isValid() || e > end ) {
                end = e;
            }
            //debugPlan<<a->node()->node()->name()<<": "<<s<<e<<": "<<m_effortMap[ a ].totalEffort().toDouble(Duration::Unit_h);
        }
        // add external appointments
        foreach (Appointment* a, r->externalAppointmentList() ) {
            extEff[ a ] = a->plannedPrDay( startDate(), endDate() );
            //debugPlan<<r->name()<<a->auxcilliaryInfo()<<": "<<extEff[ a ].totalEffort().toDouble(Duration::Unit_h);
            //debugPlan<<r->name()<<a->auxcilliaryInfo()<<": "<<extEff[ a ].startDate()<<extEff[ a ].endDate();
        }
    }
    m_effortMap.clear();
    m_externalEffortMap.clear();
    m_effortMap = ec;
    m_externalEffortMap = extEff;
    return;
}

int ResourceAppointmentsItemModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    return 3 + startDate().daysTo( endDate() );
}

int ResourceAppointmentsItemModel::rowCount( const QModelIndex &parent ) const
{
    if ( m_project == 0 || m_manager == 0 ) {
        return 0;
    }
    //debugPlan<<parent.row()<<", "<<parent.column()<<endl;
    if ( ! parent.isValid() ) {
        //debugPlan<<m_project->name()<<": "<<m_project->numResourceGroups()<<endl;
        return m_project->numResourceGroups();
    }
    ResourceGroup *g = resourcegroup( parent );
    if ( g ) {
        //debugPlan<<g->name()<<": "<<g->numResources()<<endl;
        return g->numResources();
    }
    Resource *r = resource( parent );
    if ( r ) {
        int rows = m_showInternal ? r->numAppointments( id() ) : 0;
        rows += m_showExternal ? r->numExternalAppointments() : 0;
        return rows;
    }
    return 0;
}

QVariant ResourceAppointmentsItemModel::name( const Resource *res, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return res->name();
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAppointmentsItemModel::name( const  ResourceGroup *res, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return res->name();
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAppointmentsItemModel::name( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return node->name();
            break;
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAppointmentsItemModel::name( const Appointment *app, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return app->auxcilliaryInfo();
        case Qt::ToolTipRole:
            return i18n( "External project: %1", app->auxcilliaryInfo() );
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case Qt::ForegroundRole:
            if ( m_externalEffortMap.contains( app ) ) {
                return QColor( Qt::blue );
            }
            break;
    }
    return QVariant();
}


QVariant ResourceAppointmentsItemModel::total( const Resource *res, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            Duration d;
            if ( m_showInternal ) {
                QList<Appointment*> lst = res->appointments( m_manager->scheduleId() );
                foreach ( Appointment *a, lst ) {
                    if ( m_effortMap.contains( a ) ) {
                        d += m_effortMap[ a ].totalEffort();
                    }
                }
            }
            if ( m_showExternal ) {
                QList<Appointment*> lst = res->externalAppointmentList();
                foreach ( Appointment *a, lst ) {
                    if ( m_externalEffortMap.contains( a ) ) {
                        d += m_externalEffortMap[ a ].totalEffort();
                    }
                }
            }
            return QLocale().toString( d.toDouble( Duration::Unit_h ), 'f', 1 );
        }
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case Qt::TextAlignmentRole:
            return (int)(Qt::AlignRight|Qt::AlignVCenter);
    }
    return QVariant();
}

QVariant ResourceAppointmentsItemModel::total( const Resource *res, const QDate &date, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            Duration d;
            if ( m_showInternal ) {
                QList<Appointment*> lst = res->appointments( id() );
                foreach ( Appointment *a, lst ) {
                    if ( m_effortMap.contains( a ) ) {
                        d += m_effortMap[ a ].effortOnDate( date );
                    }
                }
            }
            if ( m_showExternal ) {
                QList<Appointment*> lst = res->externalAppointmentList();
                foreach ( Appointment *a, lst ) {
                    if ( m_externalEffortMap.contains( a ) ) {
                        d += m_externalEffortMap[ a ].effortOnDate( date );
                    }
                }
            }
            QString ds = QLocale().toString( d.toDouble( Duration::Unit_h ), 'f', 1 );
            Duration avail = res->effort( 0, DateTime( date, QTime(0,0,0) ), Duration( 1.0, Duration::Unit_d ) );
            QString avails = QLocale().toString( avail.toDouble( Duration::Unit_h ), 'f', 1 );
            return QString( "%1(%2)").arg( ds).arg( avails );
        }
        case Qt::EditRole:
        case Qt::ToolTipRole:
            return i18n( "The total booking on %1, along with the maximum hours for the resource", QLocale().toString( date, QLocale::ShortFormat ) );
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case Qt::TextAlignmentRole:
            return (int)(Qt::AlignRight|Qt::AlignVCenter);
        case Qt::BackgroundRole: {
            if ( res->calendar() && res->calendar()->state( date ) != CalendarDay::Working ) {
                QColor c( 0xf0f0f0 );
                return QVariant::fromValue( c );
                //return QVariant( Qt::cyan );
            }
            break;
        }
    }
    return QVariant();
}

QVariant ResourceAppointmentsItemModel::total( const Appointment *a, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            Duration d;
            if ( m_effortMap.contains( a ) ) {
                d = m_effortMap[ a ].totalEffort();
            } else if ( m_externalEffortMap.contains( a ) ) {
                d = m_externalEffortMap[ a ].totalEffort();
            }
            return QLocale().toString( d.toDouble( Duration::Unit_h ), 'f', 1 );
        }
        case Qt::ToolTipRole: {
            if ( m_effortMap.contains( a ) ) {
                return i18n( "Total booking by this task" );
            } else if ( m_externalEffortMap.contains( a ) ) {
                return i18n( "Total booking by the external project" );
            }
            return QVariant();
        }
        case Qt::EditRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case Qt::TextAlignmentRole:
            return (int)(Qt::AlignRight|Qt::AlignVCenter);
        case Qt::ForegroundRole:
            if ( m_externalEffortMap.contains( a ) ) {
                return QColor( Qt::blue );
            }
            break;
    }
    return QVariant();
}


QVariant ResourceAppointmentsItemModel::assignment( const Appointment *a, const QDate &date, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            Duration d;
            if ( m_effortMap.contains( a ) ) {
                if ( date < m_effortMap[ a ].startDate() || date > m_effortMap[ a ].endDate() ) {
                    return QVariant();
                }
                d = m_effortMap[ a ].effortOnDate( date );
                return QLocale().toString( d.toDouble( Duration::Unit_h ), 'f', 1 );
            } else  if ( m_externalEffortMap.contains( a ) ) {
                if ( date < m_externalEffortMap[ a ].startDate() || date > m_externalEffortMap[ a ].endDate() ) {
                    return QVariant();
                }
                d = m_externalEffortMap[ a ].effortOnDate( date );
                return QLocale().toString( d.toDouble( Duration::Unit_h ), 'f', 1 );
            }
            return QVariant();
        }
        case Qt::EditRole:
        case Qt::ToolTipRole: {
            if ( m_effortMap.contains( a ) ) {
                return i18n( "Booking by this task on %1", QLocale().toString( date, QLocale::ShortFormat ) );
            } else if ( m_externalEffortMap.contains( a ) ) {
                return i18n( "Booking by external project on %1",QLocale().toString( date, QLocale::ShortFormat ) );
            }
            return QVariant();
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case Qt::TextAlignmentRole:
            return (int)(Qt::AlignRight|Qt::AlignVCenter);
        case Qt::ForegroundRole:
            if ( m_externalEffortMap.contains( a ) ) {
                return QColor( Qt::blue );
            }
            break;
        case Qt::BackgroundRole: {
            Resource *r = parent( a );
            if ( r && r->calendar() && r->calendar()->state( date ) != CalendarDay::Working ) {
                QColor c( 0xf0f0f0 );
                return QVariant::fromValue( c );
                //return QVariant( Qt::cyan );
            }
            break;
        }
    }
    return QVariant();
}

QVariant ResourceAppointmentsItemModel::notUsed( const ResourceGroup *, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole:
            return QString(" ");
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::EditRole:
        case Qt::ToolTipRole:
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
    }
    return QVariant();
}

QVariant ResourceAppointmentsItemModel::data( const QModelIndex &index, int role ) const
{
    if ( m_project == 0 || m_manager == 0 ) {
        return QVariant();
    }
    QVariant result;
    if ( index.column() >= columnCount() ) {
        debugPlan<<"invalid display value column "<<index;
        return result;
    }
    if ( ! index.isValid() ) {
        debugPlan<<"Invalid index:"<<index;
        return result;
    }
    if ( role == Qt::TextAlignmentRole ) {
        return headerData( index.column(), Qt::Horizontal, role );
    }
    Resource *r = resource( index );
    if ( r ) {
        switch ( index.column() ) {
            case 0: result = name( r, role ); break;
            case 1: result = total( r, role ); break;
            default:
                QDate d = startDate().addDays( index.column() - 2 );
                result = total( r, d, role );
                break;
        }
        if ( result.isValid() ) {
            if ( role == Qt::DisplayRole && result.type() == QVariant::String && result.toString().isEmpty()) {
                // HACK to show focus in empty cells
                result = ' ';
            }
            return result;
        }
        return QVariant();
    }
    ResourceGroup *g = resourcegroup( index );
    if ( g ) {
        switch ( index.column() ) {
            case 0: result = name( g, role ); break;
            default:
                result = notUsed( g, role );
                break;
        }
        if ( result.isValid() ) {
            if ( role == Qt::DisplayRole && result.type() == QVariant::String && result.toString().isEmpty()) {
            // HACK to show focus in empty cells
                result = ' ';
            }
            return result;
        }
        return QVariant();
    }
    Appointment *a = appointment( index );
    if ( a ) {
        switch ( index.column() ) {
            case 0: result = name( a->node()->node(), role ); break;
            case 1: result = total( a, role ); break;
            default: {
                QDate d = startDate().addDays( index.column()-2 );
                result = assignment( a, d, role );
                break;
            }
        }
        if ( result.isValid() ) {
            if ( role == Qt::DisplayRole && result.type() == QVariant::String && result.toString().isEmpty()) {
            // HACK to show focus in empty cells
                result = ' ';
            }
            return result;
        }
        return QVariant();
    }
    a = externalAppointment( index );
    if ( a ) {
        //debugPlan<<"external"<<a->auxcilliaryInfo()<<index;
        switch ( index.column() ) {
            case 0: result = name( a, role ); break;
            case 1: result = total( a, role ); break;
            default: {
                QDate d = startDate().addDays( index.column()-2 );
                result = assignment( a, d, role );
                break;
            }
        }
        if ( result.isValid() ) {
            if ( role == Qt::DisplayRole && result.type() == QVariant::String && result.toString().isEmpty()) {
            // HACK to show focus in empty cells
                result = ' ';
            }
            return result;
        }
        return QVariant();
    }
    debugPlan<<"Could not find ptr:"<<index;
    return QVariant();
}

bool ResourceAppointmentsItemModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    if ( ! index.isValid() ) {
        return ItemModelBase::setData( index, value, role );
    }
    if ( ( flags( index ) &Qt::ItemIsEditable ) == 0 || role != Qt::EditRole ) {
        return false;
    }
    Resource *r = resource( index );
    if ( r ) {
        switch (index.column()) {
            default:
                qWarning("data: invalid display value column %d", index.column());
                break;
        }
        return false;
    }
    ResourceGroup *g = resourcegroup( index );
    if ( g ) {
        switch (index.column()) {
            default:
                qWarning("data: invalid display value column %d", index.column());
                break;
        }
        return false;
    }
    return false;
}

QVariant ResourceAppointmentsItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal ) {
        if ( role == Qt::DisplayRole ) {
            switch ( section ) {
                case 0: return i18n( "Name" );
                case 1: return i18n( "Total" );
                default: {
                    //debugPlan<<section<<", "<<startDate()<<endDate();
                    if ( section < columnCount() ) {
                        QDate d = startDate().addDays( section - 2 );
                        if ( d <= endDate() ) {
                            return d;
                        }
                    }
                    return QVariant();
                }
            }
        } else if ( role == Qt::ToolTipRole ) {
            switch ( section ) {
                case 0: return i18n( "Name" );
                case 1: return i18n( "The total hours booked" );
                default: {
                    //debugPlan<<section<<", "<<startDate()<<endDate();
                    QDate d = startDate().addDays( section - 2 );
                    return i18n( "Bookings on %1", QLocale().toString( d, QLocale::ShortFormat ) );
                }
                return QVariant();
            }
        } else if ( role == Qt::TextAlignmentRole ) {
            switch (section) {
                case 0: return QVariant();
                default: return (int)(Qt::AlignRight|Qt::AlignVCenter);
            }
        }
    }
    if ( role == Qt::ToolTipRole ) {
        switch ( section ) {
            default: return QVariant();
        }
    }
    return ItemModelBase::headerData(section, orientation, role);
}

QObject *ResourceAppointmentsItemModel::object( const QModelIndex &index ) const
{
    QObject *o = 0;
    if ( index.isValid() ) {
        o = dynamic_cast<QObject*>( resource( index ) );
        if ( o ) {
            return o;
        }
        o = dynamic_cast<QObject*>( resourcegroup( index ) );
    }
    return o;
}

Node *ResourceAppointmentsItemModel::node( const QModelIndex &index ) const
{
    Appointment *a = appointment( index );
    if ( a == 0 ) {
        return 0;
    }
    return a->node()->node();
}

Appointment *ResourceAppointmentsItemModel::appointment( const QModelIndex &index ) const
{
    if ( m_project == 0 || m_manager == 0 ) {
        return 0;
    }
    foreach ( Resource *r, m_project->resourceList() ) {
        foreach ( Appointment *a, r->appointments( id() ) ) {
            if ( a == index.internalPointer() ) {
                return a;
            }
        }
    }
    return 0;
}

Appointment *ResourceAppointmentsItemModel::externalAppointment( const QModelIndex &index ) const
{
    if ( m_project == 0 || m_manager == 0 ) {
        return 0;
    }
    foreach ( Resource *r, m_project->resourceList() ) {
        foreach ( Appointment *a, r->externalAppointmentList() ) {
            if ( a == index.internalPointer() ) {
                return a;
            }
        }
    }
    return 0;
}

QModelIndex ResourceAppointmentsItemModel::createAppointmentIndex( int row, int col, void *ptr ) const
{
    return createIndex( row, col, ptr );
}

QModelIndex ResourceAppointmentsItemModel::createExternalAppointmentIndex( int row, int col, void *ptr ) const
{
    if ( m_project == 0 || m_manager == 0 ) {
        return QModelIndex();
    }
    QModelIndex i = createIndex( row, col, ptr );
    //debugPlan<<i;
    return i;
}

Resource *ResourceAppointmentsItemModel::resource( const QModelIndex &index ) const
{
    if ( m_project == 0 ) {
        return 0;
    }
    foreach ( Resource *r, m_project->resourceList() ) {
        if ( r == index.internalPointer() ) {
            return r;
        }
    }
    return 0;
}

QModelIndex ResourceAppointmentsItemModel::createResourceIndex( int row, int col, void *ptr ) const
{
    return createIndex( row, col, ptr );
}

ResourceGroup *ResourceAppointmentsItemModel::resourcegroup( const QModelIndex &index ) const
{
    if ( m_project == 0 ) {
        return 0;
    }
    foreach ( ResourceGroup *r, m_project->resourceGroups() ) {
        if ( r == index.internalPointer() ) {
            return r;
        }
    }
    return 0;
}

QModelIndex ResourceAppointmentsItemModel::createGroupIndex( int row, int col, void *ptr ) const
{
    return createIndex( row, col, ptr );
}

void ResourceAppointmentsItemModel::slotCalendarChanged( Calendar* )
{
    foreach ( Resource *r, m_project->resourceList() ) {
        if ( r->calendar( true ) == 0 ) {
            slotResourceChanged( r );
        }
    }
}

void ResourceAppointmentsItemModel::slotResourceChanged( Resource *res )
{
    ResourceGroup *g = res->parentGroup();
    if ( g ) {
        int row = g->indexOf( res );
        emit dataChanged( createResourceIndex( row, 0, res ), createResourceIndex( row, columnCount() - 1, res ) );
        return;
    }
}

void ResourceAppointmentsItemModel::slotResourceGroupChanged( ResourceGroup *res )
{
    Project *p = res->project();
    if ( p ) {
        int row = p->resourceGroups().indexOf( res );
        emit dataChanged( createGroupIndex( row, 0, res ), createGroupIndex( row, columnCount() - 1, res ) );
    }
}

//-------------------------------------------------------
class Q_DECL_HIDDEN ResourceAppointmentsRowModel::Private
{
public:
    Private( Private *par=0, void *p=0, KPlato::ObjectType t=OT_None ) : parent( par ), ptr( p ), type( t ), internalCached( false ), externalCached( false ), intervalRow( -1 )
    {}
    ~Private()
    {
        qDeleteAll( intervals );
    }

    QVariant data( int column, long id = -1, int role = Qt::DisplayRole ) const;

    Private *parent;
    void *ptr;
    KPlato::ObjectType type;
    bool internalCached;
    bool externalCached;

    Private *intervalAt( int row ) const;
    // used by interval
    AppointmentInterval interval;

protected:
    QVariant groupData( int column, int role ) const;
    QVariant resourceData( int column, long id, int role ) const;
    QVariant appointmentData( int column, int role ) const;
    QVariant externalData( int column, int role ) const;
    QVariant intervalData( int column, int role ) const;

private:
    // used by resource
    Appointment internal;
    Appointment external;

    // used by appointment
    int intervalRow;
    mutable QMap<int, Private*> intervals;
};

QVariant ResourceAppointmentsRowModel::Private::data( int column, long id, int role ) const
{
    if ( role == Role::ObjectType ) {
        return (int)type;
    }
    switch ( type ) {
        case OT_ResourceGroup: return groupData( column, role );
        case OT_Resource: return resourceData( column, id, role );
        case OT_Appointment: return appointmentData( column, role );
        case OT_External: return externalData( column, role );
        case OT_Interval: return intervalData( column, role );
        default: break;
    }
    return QVariant();
}

QVariant ResourceAppointmentsRowModel::Private::groupData( int column, int role ) const
{
    ResourceGroup *g = static_cast<ResourceGroup*>( ptr );
    if ( role == Qt::DisplayRole ) {
        switch ( column ) {
            case ResourceAppointmentsRowModel::Name: return g->name();
            case ResourceAppointmentsRowModel::Type: return g->typeToString( true );
            case ResourceAppointmentsRowModel::StartTime: return " ";
            case ResourceAppointmentsRowModel::EndTime: return " ";
            case ResourceAppointmentsRowModel::Load: return " ";
        }
    } else if ( role == Role::Maximum ) {
        return g->units(); //TODO: Maximum Load
    }
    return QVariant();
}

QVariant ResourceAppointmentsRowModel::Private::resourceData( int column, long id, int role ) const
{
    KPlato::Resource *r = static_cast<KPlato::Resource*>( ptr );
    if ( role == Qt::DisplayRole ) {
        switch ( column ) {
            case ResourceAppointmentsRowModel::Name: return r->name();
            case ResourceAppointmentsRowModel::Type: return r->typeToString( true );
            case ResourceAppointmentsRowModel::StartTime: return " ";
            case ResourceAppointmentsRowModel::EndTime: return " ";
            case ResourceAppointmentsRowModel::Load: return " ";
        }
    } else if ( role == Role::Maximum ) {
        return r->units(); //TODO: Maximum Load
    } else if ( role == Role::InternalAppointments ) {
        if ( ! internalCached ) {
            Resource *r = static_cast<Resource*>( ptr );
            const_cast<Private*>( this )->internal.clear();
            foreach ( Appointment *a, r->appointments( id ) ) {
                const_cast<Private*>( this )->internal += *a;
            }
            const_cast<Private*>( this )->internalCached = true;
        }
        return QVariant::fromValue( (void*)(&internal) );
    } else if ( role == Role::ExternalAppointments ) {
        if ( ! externalCached ) {
            Resource *r = static_cast<Resource*>( ptr );
            const_cast<Private*>( this )->external.clear();
            foreach ( Appointment *a, r->externalAppointmentList() ) {
                Appointment e;
                e.setIntervals( a->intervals( r->startTime( id ), r->endTime( id ) ) );
                const_cast<Private*>( this )->external += e;
            }
            const_cast<Private*>( this )->externalCached = true;
        }
        return QVariant::fromValue( (void*)(&external) );
    }
    return QVariant();
}

QVariant ResourceAppointmentsRowModel::Private::appointmentData( int column, int role ) const
{
    KPlato::Appointment *a = static_cast<KPlato::Appointment*>( ptr );
    if ( role == Qt::DisplayRole ) {
        switch ( column ) {
            case ResourceAppointmentsRowModel::Name: return a->node()->node()->name();
            case ResourceAppointmentsRowModel::Type: return a->node()->node()->typeToString( true );
            case ResourceAppointmentsRowModel::StartTime: return QLocale().toString( a->startTime(), QLocale::ShortFormat );
            case ResourceAppointmentsRowModel::EndTime: return QLocale().toString( a->endTime(), QLocale::ShortFormat );
            case ResourceAppointmentsRowModel::Load: return " ";
        }
    } else if ( role == Qt::ToolTipRole ) {
        Node *n = a->node()->node();
        return xi18nc( "@info:tooltip", "%1: %2<nl/>%3: %4",
                                n->wbsCode(),
                                n->name(),
                                QLocale().toString( a->startTime(), QLocale::ShortFormat ),
                                KFormat().formatDuration( ( a->endTime() - a->startTime() ).milliseconds() )
                            );
    } else if ( role == Role::Maximum ) {
        return a->resource()->resource()->units(); //TODO: Maximum Load
    }
    return QVariant();
}

QVariant ResourceAppointmentsRowModel::Private::externalData( int column, int role ) const
{
    KPlato::Appointment *a = static_cast<KPlato::Appointment*>( ptr );
    if ( role == Qt::DisplayRole ) {
        switch ( column ) {
            case ResourceAppointmentsRowModel::Name: return a->auxcilliaryInfo();
            case ResourceAppointmentsRowModel::Type: return i18n( "Project" );
            case ResourceAppointmentsRowModel::StartTime: return QLocale().toString( a->startTime(), QLocale::ShortFormat );
            case ResourceAppointmentsRowModel::EndTime: return QLocale().toString( a->endTime(), QLocale::ShortFormat );
            case ResourceAppointmentsRowModel::Load: return " ";
        }
    } else if ( role == Qt::ForegroundRole ) {
        return QColor( Qt::blue );
    } else if ( role == Role::Maximum ) {
        KPlato::Resource *r = static_cast<KPlato::Resource*>( parent->ptr );
        return r->units(); //TODO: Maximum Load
    }
    return QVariant();
}

ResourceAppointmentsRowModel::Private *ResourceAppointmentsRowModel::Private::intervalAt( int row ) const
{
    Q_ASSERT( type == OT_Appointment || type == OT_External );
    Private *p = intervals.value( row );
    if ( p  ) {
        return p;
    }
    Appointment *a = static_cast<Appointment*>( ptr );
    p = new Private( const_cast<Private*>( this ), 0, OT_Interval );
    p->intervalRow = row;
    p->interval = a->intervalAt( row );
    intervals.insert( row, p );
    return p;
}


QVariant ResourceAppointmentsRowModel::Private::intervalData( int column, int role ) const
{
    if ( role == Qt::DisplayRole ) {
        switch ( column ) {
            case ResourceAppointmentsRowModel::Name: return QVariant();
            case ResourceAppointmentsRowModel::Type: return i18n( "Interval" );
            case ResourceAppointmentsRowModel::StartTime: return QLocale().toString( interval.startTime(), QLocale::ShortFormat );
            case ResourceAppointmentsRowModel::EndTime: return QLocale().toString( interval.endTime(), QLocale::ShortFormat );
            case ResourceAppointmentsRowModel::Load: return interval.load();
        }
    } else if ( role == Qt::ToolTipRole ) {
        Appointment *a = static_cast<Appointment*>( parent->ptr );
        Node *n = a->node()->node();
        return xi18nc( "@info:tooltip", "%1: %2<nl/>%3: %4<nl/>Assigned: %5<nl/>Available: %6",
                                n->wbsCode(),
                                n->name(),
                                QLocale().toString( a->startTime(), QLocale::ShortFormat ),
                                KFormat().formatDuration( ( a->endTime() - a->startTime() ).milliseconds() ),
                                interval.load(),
                                a->resource()->resource()->units()
                            );
    } else if ( role == Role::Maximum ) {
        return parent->appointmentData( column, role );
    }
    return QVariant();
}

int ResourceAppointmentsRowModel::sortRole( int column ) const
{
    switch ( column ) {
        case ResourceAppointmentsRowModel::StartTime:
        case ResourceAppointmentsRowModel::EndTime:
            return Qt::EditRole;
        default:
            break;
    }
    return Qt::DisplayRole;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<( QDebug dbg, KPlato::ObjectType t)
{
  switch(t){
    case KPlato::OT_None:           dbg << "None"; break;
    case KPlato::OT_ResourceGroup:  dbg << "Group"; break;
    case KPlato::OT_Resource:       dbg << "Resource"; break;
    case KPlato::OT_Appointment:    dbg << "Appointment"; break;
    case KPlato::OT_External:       dbg << "External"; break;
    case KPlato::OT_Interval:       dbg << "Interval"; break;
    default: dbg << "Unknown";
  }
  return dbg;
}
QDebug operator<<( QDebug dbg, const ResourceAppointmentsRowModel::Private& s )
{
    dbg <<&s;
    return dbg;
}
QDebug operator<<( QDebug dbg, const ResourceAppointmentsRowModel::Private* s )
{
    if ( s == 0 ) {
        dbg<<"ResourceAppointmentsRowModel::Private[ ("<<(void*)s<<") ]";
    } else {
        dbg << "ResourceAppointmentsRowModel::Private[ ("<<(void*)s<<") Type="<<s->type<<" parent=";
        switch( s->type ) {
        case KPlato::OT_ResourceGroup:
            dbg<<static_cast<ResourceGroup*>(s->ptr)->project()<<static_cast<ResourceGroup*>(s->ptr)->project()->name();
            dbg<<" ptr="<<static_cast<ResourceGroup*>(s->ptr)<<static_cast<ResourceGroup*>(s->ptr)->name();
            break;
        case KPlato::OT_Resource:
            dbg<<static_cast<ResourceGroup*>(s->parent->ptr)<<static_cast<ResourceGroup*>(s->parent->ptr)->name();
            dbg<<" ptr="<<static_cast<Resource*>(s->ptr)<<static_cast<Resource*>(s->ptr)->name();
            break;
        case KPlato::OT_Appointment:
        case KPlato::OT_External:
            dbg<<static_cast<Resource*>(s->parent->ptr)<<static_cast<Resource*>(s->parent->ptr)->name();
            dbg<<" ptr="<<static_cast<Appointment*>(s->ptr);
            break;
        case KPlato::OT_Interval:
            dbg<<static_cast<Appointment*>(s->parent->ptr)<<" ptr="<<static_cast<AppointmentInterval*>(s->ptr);
            break;
        default:
            dbg<<s->parent<<" ptr="<<s->ptr;
            break;
        }
        dbg<<" ]";
    }

    return dbg;
}
#endif

ResourceAppointmentsRowModel::ResourceAppointmentsRowModel( QObject *parent )
    : ItemModelBase( parent ),
    m_schedule( 0 )
{
}

ResourceAppointmentsRowModel::~ResourceAppointmentsRowModel()
{
    qDeleteAll( m_datamap );
}

void ResourceAppointmentsRowModel::setProject( Project *project )
{
    //debugPlan<<project;
    if ( m_project ) {
        disconnect(m_project, SIGNAL(aboutToBeDeleted()), this, SLOT(projectDeleted()));

        disconnect( m_project, SIGNAL(resourceGroupToBeAdded(const ResourceGroup*,int)), this, SLOT(slotResourceGroupToBeInserted(const ResourceGroup*,int)) );

        disconnect( m_project, SIGNAL(resourceGroupToBeRemoved(const ResourceGroup*)), this, SLOT(slotResourceGroupToBeRemoved(const ResourceGroup*)) );

        disconnect( m_project, SIGNAL(resourceToBeAdded(const ResourceGroup*,int)), this, SLOT(slotResourceToBeInserted(const ResourceGroup*,int)) );

        disconnect( m_project, SIGNAL(resourceToBeRemoved(const Resource*)), this, SLOT(slotResourceToBeRemoved(const Resource*)) );

        disconnect( m_project, SIGNAL(resourceGroupAdded(const ResourceGroup*)), this, SLOT(slotResourceGroupInserted(const ResourceGroup*)) );

        disconnect( m_project, SIGNAL(resourceGroupRemoved(const ResourceGroup*)), this, SLOT(slotResourceGroupRemoved(const ResourceGroup*)) );

        disconnect( m_project, SIGNAL(resourceAdded(const Resource*)), this, SLOT(slotResourceInserted(const Resource*)) );

        disconnect( m_project, SIGNAL(resourceRemoved(const Resource*)), this, SLOT(slotResourceRemoved(const Resource*)) );

        disconnect( m_project, SIGNAL(projectCalculated(ScheduleManager*)), this, SLOT(slotProjectCalculated(ScheduleManager*)) );

        foreach ( Resource *r, m_project->resourceList() ) {
            disconnect( r, SIGNAL(externalAppointmentToBeAdded(Resource*,int)), this, SLOT(slotAppointmentToBeInserted(Resource*,int)) );
            disconnect( r, SIGNAL(externalAppointmentAdded(Resource*,Appointment*)), this, SLOT(slotAppointmentInserted(Resource*,Appointment*)) );
            disconnect( r, SIGNAL(externalAppointmentToBeRemoved(Resource*,int)), this, SLOT(slotAppointmentToBeRemoved(Resource*,int)) );
            disconnect( r, SIGNAL(externalAppointmentRemoved()), this, SLOT(slotAppointmentRemoved()) );
            disconnect( r, SIGNAL(externalAppointmentChanged(Resource*,Appointment*)), this, SLOT(slotAppointmentChanged(Resource*,Appointment*)) );
        }
    }
    m_project = project;
    if ( m_project ) {
        connect(m_project, SIGNAL(aboutToBeDeleted()), this, SLOT(projectDeleted()));

        connect( m_project, SIGNAL(resourceGroupToBeAdded(const ResourceGroup*,int)), this, SLOT(slotResourceGroupToBeInserted(const ResourceGroup*,int)) );

        connect( m_project, SIGNAL(resourceGroupToBeRemoved(const ResourceGroup*)), this, SLOT(slotResourceGroupToBeRemoved(const ResourceGroup*)) );

        connect( m_project, SIGNAL(resourceToBeAdded(const ResourceGroup*,int)), this, SLOT(slotResourceToBeInserted(const ResourceGroup*,int)) );

        connect( m_project, SIGNAL(resourceToBeRemoved(const Resource*)), this, SLOT(slotResourceToBeRemoved(const Resource*)) );

        connect( m_project, SIGNAL(resourceGroupAdded(const ResourceGroup*)), this, SLOT(slotResourceGroupInserted(const ResourceGroup*)) );

        connect( m_project, SIGNAL(resourceGroupRemoved(const ResourceGroup*)), this, SLOT(slotResourceGroupRemoved(const ResourceGroup*)) );

        connect( m_project, SIGNAL(resourceAdded(const Resource*)), this, SLOT(slotResourceInserted(const Resource*)) );

        connect( m_project, SIGNAL(resourceRemoved(const Resource*)), this, SLOT(slotResourceRemoved(const Resource*)) );

        connect( m_project, SIGNAL(projectCalculated(ScheduleManager*)), this, SLOT(slotProjectCalculated(ScheduleManager*)) );

        foreach ( Resource *r, m_project->resourceList() ) {
            connect( r, SIGNAL(externalAppointmentToBeAdded(Resource*,int)), this, SLOT(slotAppointmentToBeInserted(Resource*,int)) );
            connect( r, SIGNAL(externalAppointmentAdded(Resource*,Appointment*)), this, SLOT(slotAppointmentInserted(Resource*,Appointment*)) );
            connect( r, SIGNAL(externalAppointmentToBeRemoved(Resource*,int)), this, SLOT(slotAppointmentToBeRemoved(Resource*,int)) );
            connect( r, SIGNAL(externalAppointmentRemoved()), this, SLOT(slotAppointmentRemoved()) );
            connect( r, SIGNAL(externalAppointmentChanged(Resource*,Appointment*)), this, SLOT(slotAppointmentChanged(Resource*,Appointment*)) );
        }
    }
    reset();
}

void ResourceAppointmentsRowModel::setScheduleManager( ScheduleManager *sm )
{
    debugPlan<<"ResourceAppointmentsRowModel::setScheduleManager:"<<sm;
    if ( sm == 0 || sm != m_manager || sm->expected() != m_schedule ) {
        m_manager = sm;
        m_schedule = sm ? sm->expected() : 0;
        qDeleteAll( m_datamap );
        m_datamap.clear();
        reset();
    }
}

long ResourceAppointmentsRowModel::id() const
{
    return m_manager ? m_manager->scheduleId() : -1;
}

const QMetaEnum ResourceAppointmentsRowModel::columnMap() const
{
    return metaObject()->enumerator( metaObject()->indexOfEnumerator("Properties") );
}

int ResourceAppointmentsRowModel::columnCount( const QModelIndex & /*parent */) const
{
    return columnMap().keyCount();
}

int ResourceAppointmentsRowModel::rowCount( const QModelIndex & parent ) const
{
    if ( m_project == 0 ) {
        return 0;
    }
    if ( ! parent.isValid() ) {
        return m_project->numResourceGroups();
    }
    if ( ResourceGroup *g = resourcegroup( parent ) ) {
        return g->numResources();
    }
    if ( m_manager == 0 ) {
        return 0;
    }
    if ( Resource *r = resource( parent ) ) {
        return r->numAppointments( id() ) + r->numExternalAppointments(); // number of tasks there are appointments with + external projects
    }
    if ( Appointment *a = appointment( parent ) ) {
        return a->count(); // number of appointment intervals
    }
    return 0;
}

QVariant ResourceAppointmentsRowModel::data( const QModelIndex &index, int role ) const
{
    //debugPlan<<index<<role;
    if ( ! index.isValid() ) {
        return QVariant();
    }
    if ( role == Qt::TextAlignmentRole ) {
        return headerData( index.column(), Qt::Horizontal, role );
    }
    return static_cast<Private*>(index.internalPointer() )->data( index.column(), id(), role );
}

QVariant ResourceAppointmentsRowModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Vertical ) {
        return QVariant();
    }
    if ( role == Qt::DisplayRole ) {
        switch ( section ) {
            case Name: return i18n( "Name" );
            case Type: return i18n( "Type" );
            case StartTime: return i18n( "Start Time" );
            case EndTime: return i18n( "End Time" );
            case Load: return xi18nc( "@title:column noun", "Load" );
        }
    }
    if ( role == Qt::TextAlignmentRole ) {
        switch ( section ) {
            case Name:
            case Type:
            case StartTime:
            case EndTime:
                return (int)(Qt::AlignLeft|Qt::AlignVCenter);
            case Load:
                return (int)(Qt::AlignRight|Qt::AlignVCenter);
        }
    }
    return ItemModelBase::headerData( section, orientation, role );
}

QModelIndex ResourceAppointmentsRowModel::parent( const QModelIndex &idx ) const
{
    if ( !idx.isValid() || m_project == 0 ) {
        warnPlan<<"No data "<<idx;
        return QModelIndex();
    }

    QModelIndex p;
    if ( resourcegroup( idx ) ) {
        // ResourceGroup, no parent
        return QModelIndex();
    }
    if ( ResourceGroup *pg = parentGroup( idx ) ) {
        // Resource, parent is ResourceGroup
        int row = m_project->indexOf( pg );
        p = const_cast<ResourceAppointmentsRowModel*>( this )->createGroupIndex( row, 0, m_project );
        //debugPlan<<"Parent:"<<p<<r->parentGroup()->name();
        Q_ASSERT( p.isValid() );
        return p;
    }
    if ( Resource *pr = parentResource( idx ) ) {
        // Appointment, parent is Resource
        int row = pr->parentGroup()->indexOf( pr );
        p = const_cast<ResourceAppointmentsRowModel*>( this )->createResourceIndex( row, 0, pr->parentGroup() );
        //debugPlan<<"Parent:"<<p<<r->parentGroup()->name();
        Q_ASSERT( p.isValid() );
        return p;
    }
    if ( Appointment *a = parentAppointment( idx ) ) {
        // AppointmentInterval, parent is Appointment
        Private *pi = static_cast<Private*>( idx.internalPointer() );
        if ( pi->parent->type == OT_Appointment ) {
            Q_ASSERT( a->resource()->id() == id() );
            if ( a->resource() && a->resource()->resource() ) {
                Resource *r = a->resource()->resource();
                int row = r->indexOf( a, id() );
                Q_ASSERT( row >= 0 );
                p = const_cast<ResourceAppointmentsRowModel*>( this )->createAppointmentIndex( row, 0, r );
                //debugPlan<<"Parent:"<<p<<r->name();
                Q_ASSERT( p.isValid() );
            }
        } else if ( pi->parent->type == OT_External ) {
            Resource *r = static_cast<Resource*>( pi->parent->parent->ptr );
            int row = r->externalAppointmentList().indexOf( a );
            Q_ASSERT( row >= 0 );
            row += r->numAppointments( id() );
            p = const_cast<ResourceAppointmentsRowModel*>( this )->createAppointmentIndex( row, 0, r );
        }
        return p;
    }
    return QModelIndex();
}

QModelIndex ResourceAppointmentsRowModel::index( ResourceGroup *g ) const
{
    if ( m_project == 0 || g == 0 ) {
        return QModelIndex();
    }
    return const_cast<ResourceAppointmentsRowModel*>( this )->createGroupIndex( m_project->indexOf( g ), 0, m_project );
}

QModelIndex ResourceAppointmentsRowModel::index( Resource *r ) const
{
    if ( m_project == 0 || r == 0 ) {
        return QModelIndex();
    }
    return const_cast<ResourceAppointmentsRowModel*>( this )->createResourceIndex( r->parentGroup()->indexOf( r ), 0, r->parentGroup() );
}

QModelIndex ResourceAppointmentsRowModel::index( Appointment *a ) const
{
    if ( m_project == 0 || m_manager == 0 || a == 0 || a->resource()->resource() ) {
        return QModelIndex();
    }
    Resource *r = a->resource()->resource();
    return const_cast<ResourceAppointmentsRowModel*>( this )->createAppointmentIndex( r->indexOf( a, id() ), 0, r );
}

QModelIndex ResourceAppointmentsRowModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( m_project == 0 || row < 0 || column < 0 ) {
        return QModelIndex();
    }
    if ( ! parent.isValid() ) {
        if ( row < m_project->numResourceGroups() ) {
            //debugPlan<<"Group: "<<m_project->resourceGroupAt( row );
            return const_cast<ResourceAppointmentsRowModel*>( this )->createGroupIndex( row, column, m_project );
        }
        return QModelIndex();
    }
    if ( ResourceGroup *g = resourcegroup( parent ) ) {
        if ( row < g->numResources() ) {
            //debugPlan<<"Resource: "<<g->resourceAt( row )<<static_cast<Private*>( parent.internalPointer() );
            return const_cast<ResourceAppointmentsRowModel*>( this )->createResourceIndex( row, column, g );
        }
        return QModelIndex();
    }
    if ( m_manager == 0 ) {
        return QModelIndex();
    }
    if ( Resource *r = resource( parent ) ) {
        int num = r->numAppointments( id() ) + r->numExternalAppointments();
        if ( row < num ) {
            //debugPlan<<"Appointment: "<<r->appointmentAt( row, m_manager->scheduleId() )<<static_cast<Private*>( parent.internalPointer() );
            return const_cast<ResourceAppointmentsRowModel*>( this )->createAppointmentIndex( row, column, r );
        }
        return QModelIndex();
    }
    if ( Appointment *a = appointment( parent ) ) {
        int num = a->count();
        if ( row < num ) {
            //debugPlan<<"Appointment interval at: "<<row<<static_cast<Private*>( parent.internalPointer() );
            return const_cast<ResourceAppointmentsRowModel*>( this )->createIntervalIndex( row, column, a );
        }
        return QModelIndex();
    }
    return QModelIndex();
}

QModelIndex ResourceAppointmentsRowModel::createGroupIndex( int row, int column, Project *project )
{
    ResourceGroup *group = project->resourceGroupAt( row );
    Private *p = m_datamap.value( (void*)group );
    if ( p == 0 ) {
        p = new Private( 0, group, OT_ResourceGroup );
        m_datamap.insert( group, p );
    }
    QModelIndex idx = createIndex( row, column, p );
    Q_ASSERT( idx.isValid() );
    return idx;
}

QModelIndex ResourceAppointmentsRowModel::createResourceIndex( int row, int column, ResourceGroup *g )
{
    Resource *res = g->resourceAt( row );
    Private *p = m_datamap.value( (void*)res );
    if ( p == 0 ) {
        Private *pg = m_datamap.value( g );
        Q_ASSERT( pg );
        p = new Private( pg, res, OT_Resource );
        m_datamap.insert( res, p );
    }
    QModelIndex idx = createIndex( row, column, p );
    Q_ASSERT( idx.isValid() );
    return idx;
}

QModelIndex ResourceAppointmentsRowModel::createAppointmentIndex( int row, int column, Resource *r )
{
    Private *p = 0;
    KPlato::ObjectType type;
    Appointment *a = 0;
    if ( row < r->numAppointments( id() ) ) {
        a = r->appointmentAt( row, id() );
        type = OT_Appointment;
    } else {
        a = r->externalAppointmentList().value( row - r->numAppointments( id() ) );
        type = OT_External;
    }
    Q_ASSERT( a );
    p = m_datamap.value( (void*)a );
    if ( p == 0 ) {
        Private *pr = m_datamap.value( r );
        Q_ASSERT( pr );
        p = new Private( pr, a, type );
        m_datamap.insert( a, p );
    }
    QModelIndex idx = createIndex( row, column, p );
    Q_ASSERT( idx.isValid() );
    return idx;
}

QModelIndex ResourceAppointmentsRowModel::createIntervalIndex( int row, int column, Appointment *a )
{
    AppointmentInterval i = a->intervalAt( row );
    Private *pr = m_datamap.value( a );
    Q_ASSERT( pr );
    Private *p = pr->intervalAt( row );
    Q_ASSERT( p );

    QModelIndex idx = createIndex( row, column, p );
    Q_ASSERT( idx.isValid() );
    return idx;
}

void ResourceAppointmentsRowModel::slotResourceToBeInserted( const ResourceGroup *group, int row )
{
    debugPlan<<group->name()<<row;
    QModelIndex i = index( const_cast<ResourceGroup*>( group ) );
    beginInsertRows( i, row, row );
}

void ResourceAppointmentsRowModel::slotResourceInserted( const Resource *r )
{
    debugPlan<<r->name();
    endInsertRows();

    connect( r, SIGNAL(externalAppointmentToBeAdded(Resource*,int)), this, SLOT(slotAppointmentToBeInserted(Resource*,int)) );
    connect( r, SIGNAL(externalAppointmentAdded(Resource*,Appointment*)), this, SLOT(slotAppointmentInserted(Resource*,Appointment*)) );
    connect( r, SIGNAL(externalAppointmentToBeRemoved(Resource*,int)), this, SLOT(slotAppointmentToBeRemoved(Resource*,int)) );
    connect( r, SIGNAL(externalAppointmentRemoved()), this, SLOT(slotAppointmentRemoved()) );
    connect( r, SIGNAL(externalAppointmentChanged(Resource*,Appointment*)), this, SLOT(slotAppointmentChanged(Resource*,Appointment*)) );
}

void ResourceAppointmentsRowModel::slotResourceToBeRemoved( const Resource *r )
{
    debugPlan<<r->name();
    int row = r->parentGroup()->indexOf( r );

    beginRemoveRows( index( r->parentGroup() ), row, row );
    disconnect( r, SIGNAL(externalAppointmentToBeAdded(Resource*,int)), this, SLOT(slotAppointmentToBeInserted(Resource*,int)) );
    disconnect( r, SIGNAL(externalAppointmentAdded(Resource*,Appointment*)), this, SLOT(slotAppointmentInserted(Resource*,Appointment*)) );
    disconnect( r, SIGNAL(externalAppointmentToBeRemoved(Resource*,int)), this, SLOT(slotAppointmentToBeRemoved(Resource*,int)) );
    disconnect( r, SIGNAL(externalAppointmentRemoved()), this, SLOT(slotAppointmentRemoved()) );
    disconnect( r, SIGNAL(externalAppointmentChanged(Resource*,Appointment*)), this, SLOT(slotAppointmentChanged(Resource*,Appointment*)) );

    Private *p = 0;
    foreach ( Appointment *a, r->appointments( id() ) ) {
        // remove appointment
        p = m_datamap.value( a );
        if ( p ) {
            m_datamap.remove( a );
            delete p;
        }
    }
    foreach ( Appointment *a, r->externalAppointmentList() ) {
        // remove appointment
        p = m_datamap.value( a );
        if ( p ) {
            m_datamap.remove( a );
            delete p;
        }
    }
    // remove resource
    p = m_datamap.value( (void*)r );
    if ( p ) {
        m_datamap.remove( const_cast<Resource*>( r ) );
        delete p;
    }
}

void ResourceAppointmentsRowModel::slotResourceRemoved( const Resource *resource )
{
    Q_UNUSED(resource);
    //debugPlan<<resource->name();
    endRemoveRows();
}

void ResourceAppointmentsRowModel::slotResourceGroupToBeInserted( const ResourceGroup *group, int row )
{
    Q_UNUSED(group);
    beginInsertRows( QModelIndex(), row, row );
}

void ResourceAppointmentsRowModel::slotResourceGroupInserted( const ResourceGroup*/*group*/ )
{
    endInsertRows();
}

void ResourceAppointmentsRowModel::slotResourceGroupToBeRemoved( const ResourceGroup *group )
{
    //debugPlan<<group->name()<<endl;
    int row = m_project->indexOf( const_cast<ResourceGroup*>( group ) );
    beginRemoveRows( QModelIndex(), row, row );

    Private *p = m_datamap.value( const_cast<ResourceGroup*>( group ) );
    if ( p ) {
        m_datamap.remove( const_cast<ResourceGroup*>( group ) );
        delete p;
    }
}

void ResourceAppointmentsRowModel::slotResourceGroupRemoved( const ResourceGroup *group )
{
    Q_UNUSED(group);
    //debugPlan<<group->name();
    endRemoveRows();
}

void ResourceAppointmentsRowModel::slotAppointmentToBeInserted( Resource *r, int row )
{
    Q_UNUSED(r);
    Q_UNUSED(row);
    // external appointments only, (Internal handled in slotProjectCalculated)
}

void ResourceAppointmentsRowModel::slotAppointmentInserted( Resource *r, Appointment *a )
{
    Q_UNUSED(a);
    // external appointments only, (Internal handled in slotProjectCalculated)
    Private *p = m_datamap.value( r );
    if ( p ) {
        p->externalCached = false;
    }
    reset();
}

void ResourceAppointmentsRowModel::slotAppointmentToBeRemoved( Resource *r, int row )
{
    Q_UNUSED(row);
    // external appointments only, (Internal handled in slotProjectCalculated)
    Private *p = m_datamap.value( r );
    if ( p ) {
        p->externalCached = false;
    }
}

void ResourceAppointmentsRowModel::slotAppointmentRemoved()
{
    // external appointments only, (Internal handled in slotProjectCalculated)
    reset();
}

void ResourceAppointmentsRowModel::slotAppointmentChanged( Resource *r, Appointment *a )
{
    Q_UNUSED(r);
    Q_UNUSED(a);
    // external appointments only, (Internal handled in slotProjectCalculated)
    // will not happen atm
}

void ResourceAppointmentsRowModel::slotProjectCalculated( ScheduleManager *sm )
{
    if ( sm == m_manager ) {
        setScheduleManager( sm );
    }
}

ResourceGroup *ResourceAppointmentsRowModel::parentGroup( const QModelIndex &index ) const
{
    if ( m_project == 0 ) {
        return 0;
    }
    Private *ch = static_cast<Private*>( index.internalPointer() );
    if ( ch && ch->type == OT_Resource ) {
        return static_cast<ResourceGroup*>( ch->parent->ptr );
    }
    return 0;
}

ResourceGroup *ResourceAppointmentsRowModel::resourcegroup( const QModelIndex &index ) const
{
    if ( m_project == 0 ) {
        return 0;
    }
    Private *p = static_cast<Private*>( index.internalPointer() );
    if ( p && p->type == OT_ResourceGroup ) {
        return static_cast<ResourceGroup*>( p->ptr );
    }
    return 0;
}

Resource *ResourceAppointmentsRowModel::parentResource( const QModelIndex &index ) const
{
    if ( m_project == 0 ) {
        return 0;
    }
    Private *ch = static_cast<Private*>( index.internalPointer() );
    if ( ch && ( ch->type == OT_Appointment || ch->type == OT_External ) ) {
        return static_cast<Resource*>( ch->parent->ptr );
    }
    return 0;
}


Resource *ResourceAppointmentsRowModel::resource( const QModelIndex &index ) const
{
    if ( m_project == 0 ) {
        return 0;
    }
    Private *p = static_cast<Private*>( index.internalPointer() );
    if ( p && p->type == OT_Resource ) {
        return static_cast<Resource*>( p->ptr );
    }
    return 0;
}

Appointment *ResourceAppointmentsRowModel::parentAppointment( const QModelIndex &index ) const
{
    if ( m_project == 0 || m_manager == 0 ) {
        return 0;
    }
    Private *ch = static_cast<Private*>( index.internalPointer() );
    if ( ch && ch->type == OT_Interval ) {
        return static_cast<Appointment*>( ch->parent->ptr );
    }
    return 0;
}

Appointment *ResourceAppointmentsRowModel::appointment( const QModelIndex &index ) const
{
    if ( m_project == 0 || m_manager == 0 || ! index.isValid() ) {
        return 0;
    }
    Private *p = static_cast<Private*>( index.internalPointer() );
    if ( p && ( p->type == OT_Appointment || p->type == OT_External ) ) {
        return static_cast<Appointment*>( p->ptr );
    }
    return 0;
}

AppointmentInterval *ResourceAppointmentsRowModel::interval( const QModelIndex &index ) const
{
    if ( m_project == 0 || m_manager == 0 ) {
        return 0;
    }
    Private *p = static_cast<Private*>( index.internalPointer() );
    if ( p && p->type == OT_Interval ) {
        return &( p->interval );
    }
    return 0;
}

Node *ResourceAppointmentsRowModel::node( const QModelIndex &idx ) const
{
    Appointment *a = appointment( idx );
    return ( a && a->node() ? a->node()->node() : 0 );
}


//---------------------------------------------
ResourceAppointmentsGanttModel::ResourceAppointmentsGanttModel( QObject *parent )
    : ResourceAppointmentsRowModel( parent )
{
}

ResourceAppointmentsGanttModel::~ResourceAppointmentsGanttModel()
{
}

QVariant ResourceAppointmentsGanttModel::data( const ResourceGroup *g, int column, int role ) const
{
    Q_UNUSED(column);
    switch( role ) {
        case KGantt::ItemTypeRole: return KGantt::TypeSummary;
        case KGantt::StartTimeRole: return g->startTime( id() );
        case KGantt::EndTimeRole: return g->endTime( id() );
    }
    return QVariant();
}

QVariant ResourceAppointmentsGanttModel::data( const Resource *r, int column, int role ) const
{
    Q_UNUSED(column);
    switch( role ) {
        case KGantt::ItemTypeRole: return KGantt::TypeSummary;
        case KGantt::StartTimeRole: return r->startTime( id() );
        case KGantt::EndTimeRole: return r->endTime( id() );
    }
    return QVariant();
}

QVariant ResourceAppointmentsGanttModel::data( const Appointment *a, int column, int role ) const
{
    Q_UNUSED(column);
    switch( role ) {
        case KGantt::ItemTypeRole: return KGantt::TypeMulti;
        case KGantt::StartTimeRole: return a->startTime();
        case KGantt::EndTimeRole: return a->endTime();
    }
    return QVariant();
}

QVariant ResourceAppointmentsGanttModel::data( const AppointmentInterval *a, int column, int role ) const
{
    Q_UNUSED(column);
    switch( role ) {
        case KGantt::ItemTypeRole: return KGantt::TypeTask;
        case KGantt::StartTimeRole: return a->startTime();
        case KGantt::EndTimeRole: return a->endTime();
    }
    return QVariant();
}

QVariant ResourceAppointmentsGanttModel::data( const QModelIndex &index, int role ) const
{
    //debugPlan<<index<<role;
    if ( m_project == 0 || ! index.isValid() ) {
        return QVariant();
    }
    if ( role == KGantt::ItemTypeRole ||
         role == KGantt::StartTimeRole ||
         role == KGantt::EndTimeRole ||
         role == KGantt::TaskCompletionRole )
    {
        if ( ResourceGroup *g = resourcegroup( index ) ) {
            return data( g, index.column(), role );
        }
        if ( Resource *r = resource( index ) ) {
            return data( r, index.column(), role );
        }
        if ( m_manager == 0 ) {
            return QVariant();
        }
        if ( Appointment *a = appointment( index ) ) {
            return data( a, index.column(), role );
        }
        if ( AppointmentInterval *i = interval( index ) ) {
            return data( i, index.column(), role );
        }
        return QVariant();
    }
    return ResourceAppointmentsRowModel::data( index, role );
}

} // namespace KPlato
