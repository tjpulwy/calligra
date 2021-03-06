/* This file is part of the KDE project
  Copyright (C) 2007 Dag Andersen <danders@get2net.dk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version..

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#ifndef NODEITEMMODEL_H
#define NODEITEMMODEL_H

#include "kptitemmodelbase.h"
#include "kptschedule.h"
#include "kptworkpackagemodel.h"

#include <QDate>
#include <QMetaEnum>
#include <QSortFilterProxyModel>

class QUrl;
class QMimeData;
class KUndo2Command;
class KoXmlWriter;
class KoStore;
class KoOdfWriteStore;

namespace KPlato
{

class Project;
class Node;
class Estimate;

class KPLATOMODELS_EXPORT NodeModel : public QObject
{
    Q_OBJECT
    Q_ENUMS( Properties )
public:
    NodeModel();
    ~NodeModel() {}
    
    enum SpecialRoles {
        SortableRole = Qt::UserRole + 5024 // unlikely high number
    };

    enum Properties {
        NodeName = 0,
        NodeType,
        NodeResponsible,
        NodeAllocation,
        NodeEstimateType,
        NodeEstimateCalendar,
        NodeEstimate,
        NodeOptimisticRatio,
        NodePessimisticRatio,
        NodeRisk,
        NodeConstraint,
        NodeConstraintStart,
        NodeConstraintEnd,
        NodeRunningAccount,
        NodeStartupAccount,
        NodeStartupCost,
        NodeShutdownAccount,
        NodeShutdownCost,
        NodeDescription,

        // Based on edited values
        NodeExpected,
        NodeVarianceEstimate,
        NodeOptimistic,
        NodePessimistic,

        // After scheduling
        NodeStartTime,
        NodeEndTime,
        NodeEarlyStart,
        NodeEarlyFinish,
        NodeLateStart,
        NodeLateFinish,
        NodePositiveFloat,
        NodeFreeFloat,
        NodeNegativeFloat,
        NodeStartFloat,
        NodeFinishFloat,
        NodeAssignments,

        // Based on scheduled values
        NodeDuration,
        NodeVarianceDuration,
        NodeOptimisticDuration,
        NodePessimisticDuration,

        // Completion
        NodeStatus,
        NodeCompleted,
        NodePlannedEffort,
        NodeActualEffort,
        NodeRemainingEffort,
        NodePlannedCost,
        NodeActualCost,
        NodeActualStart,
        NodeStarted,
        NodeActualFinish,
        NodeFinished,
        NodeStatusNote,
            
        // Scheduling errors
        NodeSchedulingStatus,
        NodeNotScheduled,
        NodeAssignmentMissing,
        NodeResourceOverbooked,
        NodeResourceUnavailable,
        NodeConstraintsError,
        NodeEffortNotMet,
        NodeSchedulingError,

        NodeWBSCode,
        NodeLevel,
        
        // Performance
        NodeBCWS,
        NodeBCWP,
        NodeACWP,
        NodePerformanceIndex,
        //
        NodeCritical,
        NodeCriticalPath,

        // Info from latest work package transmission
        WPOwnerName,
        WPTransmitionStatus,
        WPTransmitionTime
    };
    const QMetaEnum columnMap() const;
    
    void setProject( Project *project );
    void setManager( ScheduleManager *sm );
    Project *project() const { return m_project; }
    ScheduleManager *manager() const { return m_manager; }
    long id() const { return m_manager == 0 ? -1 : m_manager->scheduleId(); }
    
    QVariant data( const Node *node, int property, int role = Qt::DisplayRole ) const; 
    KUndo2Command *setData( Node *node, int property, const QVariant & value, int role = Qt::EditRole );
    
    static QVariant headerData( int section, int role = Qt::DisplayRole );

    int propertyCount() const;
    
    void setNow( const QDate &now ) { m_now = now; }
    QDate now() const { return m_now; }
    
    QVariant name( const Node *node, int role ) const;
    QVariant leader( const Node *node, int role ) const;
    QVariant allocation( const Node *node, int role ) const;
    QVariant description( const Node *node, int role ) const;
    QVariant type( const Node *node, int role ) const;
    QVariant constraint( const Node *node, int role ) const;
    QVariant constraintStartTime( const Node *node, int role ) const;
    QVariant constraintEndTime( const Node *node, int role ) const;
    QVariant estimateType( const Node *node, int role ) const;
    QVariant estimateCalendar( const Node *node, int role ) const;
    QVariant estimate( const Node *node, int role ) const;
    QVariant optimisticRatio( const Node *node, int role ) const;
    QVariant pessimisticRatio( const Node *node, int role ) const;
    QVariant riskType( const Node *node, int role ) const;
    QVariant runningAccount( const Node *node, int role ) const;
    QVariant startupAccount( const Node *node, int role ) const;
    QVariant startupCost( const Node *node, int role ) const;
    QVariant shutdownAccount( const Node *node, int role ) const;
    QVariant shutdownCost( const Node *node, int role ) const;
    
    QVariant startTime( const Node *node, int role ) const;
    QVariant endTime( const Node *node, int role ) const;

    QVariant duration( const Node *node, int role ) const;
    QVariant varianceDuration( const Node *node, int role ) const;
    QVariant varianceEstimate( const Estimate *est, int role ) const;
    QVariant optimisticDuration( const Node *node, int role ) const;
    QVariant optimisticEstimate( const Estimate *est, int role ) const;
    QVariant pertExpected( const Estimate *est, int role ) const;
    QVariant pessimisticDuration( const Node *node, int role ) const;
    QVariant pessimisticEstimate( const Estimate *est, int role ) const;

    QVariant earlyStart( const Node *node, int role ) const;
    QVariant earlyFinish( const Node *node, int role ) const;
    QVariant lateStart( const Node *node, int role ) const;
    QVariant lateFinish( const Node *node, int role ) const;
    QVariant positiveFloat( const Node *node, int role ) const;
    QVariant freeFloat( const Node *node, int role ) const;
    QVariant negativeFloat( const Node *node, int role ) const;
    QVariant startFloat( const Node *node, int role ) const;
    QVariant finishFloat( const Node *node, int role ) const;
    QVariant assignedResources( const Node *node, int role ) const;
    
    QVariant status( const Node *node, int role ) const;
    QVariant completed( const Node *node, int role ) const;
    QVariant startedTime( const Node *node, int role ) const;
    QVariant isStarted( const Node *node, int role ) const;
    QVariant finishedTime( const Node *node, int role ) const;
    QVariant isFinished( const Node *node, int role ) const;
    QVariant plannedEffortTo( const Node *node, int role ) const;
    QVariant actualEffortTo( const Node *node, int role ) const;
    QVariant remainingEffort( const Node *node, int role ) const;
    QVariant plannedCostTo( const Node *node, int role ) const;
    QVariant actualCostTo( const Node *node, int role ) const;
    QVariant note( const Node *node, int role ) const;

    /// The nodes scheduling status
    QVariant nodeSchedulingStatus( const Node *node, int role ) const;
    /// Set if the node has not been scheduled
    QVariant nodeIsNotScheduled( const Node *node, int role ) const;
    /// Set if EffortType == Effort, but no resource is requested
    QVariant resourceIsMissing( const Node *node, int role ) const;
    /// Set if the assigned resource is overbooked
    QVariant resourceIsOverbooked( const Node *node, int role ) const;
    /// Set if the requested resource is not available
    QVariant resourceIsNotAvailable( const Node *node, int role ) const;
    /// Set if the task cannot be scheduled to fullfil all the constraints
    QVariant schedulingConstraintsError( const Node *node, int role ) const;
    /// Resources could not fullfil estimate
    QVariant effortNotMet( const Node *node, int role ) const;
    /// Other scheduling error occurred
    QVariant schedulingError( const Node *node, int role ) const;

    QVariant wbsCode( const Node *node, int role ) const;
    QVariant nodeLevel( const Node *node, int role ) const;
    
    QVariant nodeBCWS( const Node *node, int role ) const;
    QVariant nodeBCWP( const Node *node, int role ) const;
    QVariant nodeACWP( const Node *node, int role ) const;
    QVariant nodePerformanceIndex( const Node *node, int role ) const;

    QVariant nodeIsCritical( const Node *node, int role ) const;
    QVariant nodeInCriticalPath( const Node *node, int role ) const;

    QVariant wpOwnerName( const Node *node, int role ) const;
    QVariant wpTransmitionStatus( const Node *node, int role ) const;
    QVariant wpTransmitionTime( const Node *node, int role ) const;

    KUndo2Command *setName( Node *node, const QVariant &value, int role );
    KUndo2Command *setLeader( Node *node, const QVariant &value, int role );
    KUndo2Command *setAllocation( Node *node, const QVariant &value, int role );
    KUndo2Command *setDescription( Node *node, const QVariant &value, int role );
    KUndo2Command *setType( Node *node, const QVariant &value, int role );
    KUndo2Command *setConstraint( Node *node, const QVariant &value, int role );
    KUndo2Command *setConstraintStartTime( Node *node, const QVariant &value, int role );
    KUndo2Command *setConstraintEndTime( Node *node, const QVariant &value, int role );
    KUndo2Command *setEstimateType( Node *node, const QVariant &value, int role );
    KUndo2Command *setEstimateCalendar( Node *node, const QVariant &value, int role );
    KUndo2Command *setEstimate( Node *node, const QVariant &value, int role );
    KUndo2Command *setOptimisticRatio( Node *node, const QVariant &value, int role );
    KUndo2Command *setPessimisticRatio( Node *node, const QVariant &value, int role );
    KUndo2Command *setRiskType( Node *node, const QVariant &value, int role );
    KUndo2Command *setRunningAccount( Node *node, const QVariant &value, int role );
    KUndo2Command *setStartupAccount( Node *node, const QVariant &value, int role );
    KUndo2Command *setStartupCost( Node *node, const QVariant &value, int role );
    KUndo2Command *setShutdownAccount( Node *node, const QVariant &value, int role );
    KUndo2Command *setShutdownCost( Node *node, const QVariant &value, int role );
    KUndo2Command *setCompletion( Node *node, const QVariant &value, int role );
    KUndo2Command *setActualEffort( Node *node, const QVariant &value, int role );
    KUndo2Command *setRemainingEffort( Node *node, const QVariant &value, int role );
    KUndo2Command *setStartedTime( Node *node, const QVariant &value, int role );
    KUndo2Command *setFinishedTime( Node *node, const QVariant &value, int role );

private:
    Project *m_project;
    ScheduleManager *m_manager;
    QDate m_now;
    int m_prec;
};

class KPLATOMODELS_EXPORT NodeItemModel : public ItemModelBase
{
    Q_OBJECT
public:
    explicit NodeItemModel( QObject *parent = 0 );
    ~NodeItemModel();
    
    /// Returns a column number/- name map for this model
    virtual const QMetaEnum columnMap() const { return m_nodemodel.columnMap(); }
    
    ScheduleManager *manager() const { return m_nodemodel.manager(); }
    long id() const { return m_nodemodel.id(); }

    virtual Qt::ItemFlags flags( const QModelIndex & index ) const;
    
    virtual QModelIndex parent( const QModelIndex & index ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex index( const Node *node, int column = 0 ) const;
    
    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const; 
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const; 
    
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const; 
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );

    
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    
    virtual QMimeData * mimeData( const QModelIndexList & indexes ) const;
    virtual QStringList mimeTypes () const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    Node *node( const QModelIndex &index ) const;
    QAbstractItemDelegate *createDelegate( int column, QWidget *parent ) const;

    QModelIndex insertTask( Node *node, Node *after );
    QModelIndex insertSubtask( Node *node, Node *parent );
    
    QList<Node*> nodeList( QDataStream &stream );
    QList<Resource*> resourceList( QDataStream &stream );
    static QList<Node*> removeChildNodes( const QList<Node*> &nodes );
    bool dropAllowed( Node *on, const QMimeData *data );
    
    virtual bool dropAllowed( const QModelIndex &index, int dropIndicatorPosition, const QMimeData *data );
    
    bool projectShown() const { return m_projectshown; }

    /// Return the sortorder to be used for @p column
    virtual int sortRole( int column ) const;

Q_SIGNALS:
    void nodeInserted( Node *node );
    void projectShownChanged( bool );

public Q_SLOTS:
    virtual void setProject( Project *project );
    virtual void setScheduleManager( ScheduleManager *sm );
    void setShowProject( bool on );

protected Q_SLOTS:
    virtual void slotWbsDefinitionChanged();
    virtual void slotNodeChanged( Node* );
    virtual void slotNodeToBeInserted( Node *node, int row );
    virtual void slotNodeInserted( Node *node );
    virtual void slotNodeToBeRemoved( Node *node );
    virtual void slotNodeRemoved( Node *node );

    virtual void slotNodeToBeMoved( Node *node, int pos, Node *newParent, int newPos );
    virtual void slotNodeMoved( Node *node );

    virtual void slotLayoutChanged();
    virtual void slotProjectCalculated( ScheduleManager *sm );

protected:
    virtual bool setType( Node *node, const QVariant &value, int role );
    bool setCompletion( Node *node, const QVariant &value, int role );
    bool setAllocation( Node *node, const QVariant &value, int role );

    bool dropResourceMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );
    bool dropProjectMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );
    KUndo2Command *createAllocationCommand( Task &task, const QList<Resource*> &lst );
    bool dropUrlMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );
    bool importProjectFile( const QUrl &url, Qt::DropAction action, int row, int column, const QModelIndex &parent );

protected:
    Node *m_node; // for sanety check
    NodeModel m_nodemodel;
    bool m_projectshown;
};

//--------------------------------------
class KPLATOMODELS_EXPORT GanttItemModel : public NodeItemModel
{
    Q_OBJECT
public:
    enum GanttModelRoles { SpecialItemTypeRole = Qt::UserRole + 123 }; //FIXME

    explicit GanttItemModel(QObject *parent = 0);
    ~GanttItemModel();

    virtual int rowCount( const QModelIndex &parent ) const;
    using NodeItemModel::index;
    virtual QModelIndex index( int row, int column, const QModelIndex &parent ) const;
    QModelIndex parent( const QModelIndex &idx ) const;
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    void setShowSpecial( bool on ) { m_showSpecial = on; }
    bool showSpecial() const { return m_showSpecial; }

private:
    bool m_showSpecial;
    QMap<Node*, void*> parentmap;
};

// TODO: Rename, this is now a flat node item model
class KPLATOMODELS_EXPORT MilestoneItemModel : public ItemModelBase
{
    Q_OBJECT
public:
    explicit MilestoneItemModel( QObject *parent = 0 );
    ~MilestoneItemModel();

    /// Returns a column number/- name map for this model
    virtual const QMetaEnum columnMap() const { return m_nodemodel.columnMap(); }

    ScheduleManager *manager() const { return m_nodemodel.manager(); }
    long id() const { return m_nodemodel.id(); }

    virtual Qt::ItemFlags flags( const QModelIndex & index ) const;

    virtual QModelIndex parent( const QModelIndex & index ) const;
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex index( const Node *node ) const;

    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const; 
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const; 

    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const; 
    virtual bool setData( const QModelIndex & index, const QVariant & value, int role = Qt::EditRole );


    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    virtual QMimeData * mimeData( const QModelIndexList & indexes ) const;
    virtual QStringList mimeTypes () const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

    Node *node( const QModelIndex &index ) const;
    virtual QAbstractItemDelegate *createDelegate( int column, QWidget *parent ) const;

    QModelIndex insertTask( Node *node, Node *after );
    QModelIndex insertSubtask( Node *node, Node *parent );

    QList<Node*> nodeList( QDataStream &stream );
    static QList<Node*> removeChildNodes( const QList<Node*> &nodes );
    bool dropAllowed( Node *on, const QMimeData *data );

    virtual bool dropAllowed( const QModelIndex &index, int dropIndicatorPosition, const QMimeData *data );

    QList<Node*> mileStones() const;

    int sortRole(int column) const;

public Q_SLOTS:
    virtual void setProject( Project *project );
    virtual void setScheduleManager( ScheduleManager *sm );

protected Q_SLOTS:
    void slotNodeChanged( Node* );
    void slotNodeToBeInserted( Node *node, int row );
    void slotNodeInserted( Node *node );
    void slotNodeToBeRemoved( Node *node );
    void slotNodeRemoved( Node *node );
    void slotNodeToBeMoved( Node *node, int pos, Node *newParent, int newPos );
    void slotNodeMoved( Node *node );

    void slotLayoutChanged();
    void slotWbsDefinitionChanged();

protected:
    bool resetData();
    void resetModel();
    
private:
    NodeModel m_nodemodel;
    QMap<QString, Node*> m_nodemap;
};

class KPLATOMODELS_EXPORT NodeSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    NodeSortFilterProxyModel( ItemModelBase* model, QObject *parent, bool filterUnscheduled = true );

    ItemModelBase *itemModel() const;
    void setFilterUnscheduled( bool on );
    bool filterUnscheduled() const { return m_filterUnscheduled; }

    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

protected:
    bool filterAcceptsRow ( int source_row, const QModelIndex & source_parent ) const;

private:
    NodeItemModel *m_model;
    bool m_filterUnscheduled;
};

class KPLATOMODELS_EXPORT TaskModuleModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TaskModuleModel(QObject *parent = 0);

    void addTaskModule( Project *project );

    Qt::ItemFlags flags( const QModelIndex &idx ) const;
    int columnCount( const QModelIndex &idx = QModelIndex() ) const;
    int rowCount( const QModelIndex &idx = QModelIndex() ) const;
    QVariant data( const QModelIndex &idx, int role = Qt::DisplayRole ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    QModelIndex parent( const QModelIndex &idx ) const;
    QModelIndex index( int row, int column, const QModelIndex &parent ) const;
    QStringList mimeTypes() const;
    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );
    QMimeData *mimeData( const QModelIndexList &idx ) const;

    bool importProject( const QUrl &url, bool emitsignal = true );

public Q_SLOTS:
    void loadTaskModules( const QStringList &files );

Q_SIGNALS:
    void executeCommand( KUndo2Command *cmd );
    void saveTaskModule( const QUrl &url, Project *project );
    void removeTaskModule( const QUrl &url );

protected:
    void stripProject( Project *project ) const;

private:
    QList<Project*> m_modules;
};

} //namespace KPlato

#endif //NODEITEMMODEL_H
