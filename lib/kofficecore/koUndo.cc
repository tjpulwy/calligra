/*
*/

#include "koUndo.h"

KoCommandHistory::KoCommandHistory()
  : m_current( -1 )
{
  m_history.setAutoDelete( true );
}

void KoCommandHistory::addCommand( KoCommand* _command )
{
  if( m_current < static_cast<int> ( m_history.count() ) )
  {
    QList<KoCommand> commands;
    commands.setAutoDelete( false );

    for( int i = 0; i < m_current; i++ )
    {
      commands.insert( i, m_history.at( 0 ) );
      m_history.take( 0 );
    }

    commands.append( _command );
    m_history.clear();
    m_history = commands;
    m_history.setAutoDelete( true );
  }
  else
    m_history.append( _command );

  if( m_history.count() > MAX_UNDO_REDO )
    m_history.removeFirst();
  else
    m_current++;

  emit undoRedoChanged( getUndoName(), getRedoName() );
}

void KoCommandHistory::undo()
{
  if( m_current > 0 )
  {
    m_history.at( m_current - 1 )->unexecute();
    m_current--;
    emit undoRedoChanged( getUndoName(), getRedoName() );
  }
}

void KoCommandHistory::redo()
{
  if( m_current < static_cast<int> ( m_history.count() ) &&
      m_current > -1 )
  {
    m_history.at( m_current )->execute();
    m_current++;
    emit undoRedoChanged( getUndoName(), getRedoName() );
  }
}

QString KoCommandHistory::getUndoName()
{
  if( m_current > 0 )
    return m_history.at( m_current - 1 )->name();
  else
    return QString();
}

QString KoCommandHistory::getRedoName()
{
  if( m_current < static_cast<int> ( m_history.count() ) &&
      m_current > -1 )
    return m_history.at( m_current )->name();
  else
    return QString();
}

#include "koUndo.moc"

