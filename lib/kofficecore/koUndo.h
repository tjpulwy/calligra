/*
*/

#ifndef __koffice_undo_h__
#define __koffice_undo_h__

#include <qstring.h>
#include <qlist.h>
#include <qobject.h>

#define MAX_UNDO_REDO 100

/**
 *  The KoCommand class is an abstract command.
 *  This class has to be derived to use it.
 *
 *  @see KoCommandHistory
 *
 *  @short The KoCommand class is an abstract command.
 *  @author Michael Koch <koch@kde.orh>
 */
class KoCommand
{
public:

  /**
   *  Constructor.
   *
   *  @param _name Name of the command. This name is normally shown in the menu after "Undo" and "Redo".
   */
  KoCommand( QString _name )
  : m_name( _name ) {}

  /**
   *  Destructor.
   */   
  virtual ~KoCommand() {}

  /**
   *  Executes the command.
   *
   *  This method has to be overridden by each derived class.
   */
  virtual void execute() = 0;

  /**
   *  Makes the command undone.
   *
   *  This method has to be overridden by each derived class.
   */
  virtual void unexecute() = 0;

  QString name()
  { return m_name; }

private:

  QString m_name;
};

/**
 *  The KoCommandHistory class provides undo/redo functionality
 *  for all KOffice-apps. It uses derived classes from KoCommand.
 *
 *  @see KoCommand
 * 
 *  @short The KoCommandHistory class provides undo/redo functionality.
 *  @author Michael Koch <koch@kde.org>
 */
class KoCommandHistory : public QObject
{
  Q_OBJECT

public:

  /**
   *  Constructor.
   */
  KoCommandHistory();

  /**
   *  Adds a class derived from KoCommand to the history.
   *
   *  @param _command The command that shall be added.
   *
   *  @see KoCommand
   */
  void addCommand( KoCommand* _command );

  /**
   *  Makes the last command undone.
   */
  void undo();

  /**
   *  Redoes the last command.
   */
  void redo();

  /**
   *  Retrieves the name of the actual command that can be undone.
   *
   *  This name should be present in the menu.
   *
   *  @see getRedoName, redo, undo
   */
  QString getUndoName();

  /**
   *  Retrieves the name of the actual command that can be redone.
   *
   *  This name should be present in the menu.
   *
   *  @see getUndoName, redo, undo
   */
  QString getRedoName();

private:

  QList<KoCommand> m_history;
  int m_current;

signals:

  void undoRedoChanged( QString, QString );
};

#endif // __koffice_undo_h__
