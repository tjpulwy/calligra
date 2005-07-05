/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>
   Copyright (C) 2005 Jaroslaw Staniek <js@iidea.pl>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef FORMMANAGER_H
#define FORMMANAGER_H

#include <qobject.h>
#include <qdom.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qguardedptr.h>
#include <qstringlist.h>

class QWidget;
class QWorkspace;
class KPopupMenu;
class KActionCollection;
class KAction;
class KToggleAction;
class KDialogBase;
class KTextEdit;
class KXMLGUIClient;
class KMainWindow;

namespace KoProperty {
	class Editor;
	class Set;
	class Property;
	class Widget;
}

namespace KFormDesigner {

class WidgetPropertySet;
class Form;
class Container;
class WidgetLibrary;
class ObjectTreeView;
class Connection;
class PropertyFactory;
typedef QPtrList<KAction> ActionList;

//! A class to manage (create/load/save) Forms
/** This is Form Designer's main class, which is used by external APIs to access FormDesigner.
   This is the class you have to use to integrate FormDesigner into another program.
   It deals with creating, saving and loading Form, as well as widget insertion and copying.
   It also ensures all the components (ObjectTreeView, Form and PropertyEditor) are synced,
   and link them.
   It holds the WidgetLibrary, the WidgetPropertySet, links to ObjectTreeView and PropertyEditor,
   as well as the copied widget and the insert state.
 **/
class KFORMEDITOR_EXPORT FormManager : public QObject
{
	Q_OBJECT

	public:
		/*! Constructs FormManager object.
		 See WidgetLibrary's constructor documentation for information about
		 \a supportedFactoryGroups parameter.
		 Using \a options you can control manager's behaviour, see \ref Options. */
		FormManager(QObject *parent = 0, const QStringList& supportedFactoryGroups = QStringList(),
			int options = 0, const char *name = 0);

		virtual ~FormManager();

		enum Options { HideEventsInPopupMenu = 1, SkipFileActions = 2,
			HideSignalSlotConnections = 4 }; //todo

		/*! Creates all the KAction related to widget insertion, and plug them
		  into the KActionCollection \a parent.
		  These actions are automatically connected to \ref insertWidget() slot.
		  \return a QPtrList of the created actions.
		 */
		ActionList createActions(KActionCollection *parent);

		/*! Enables or disables actions \a name.
		 KFD uses KPart's, action collection here.
		 Kexi implements this to get (shared) actions defined elsewhere. */
		virtual void enableAction( const char* name, bool enable ) = 0;

		/*! \return action for \a name. @see enableAction() */
		virtual KAction* action(const char* name) = 0;

		bool isPasteEnabled();

		//! \return A pointer to the WidgetLibrary owned by this Manager.
		WidgetLibrary* lib() const { return m_lib; }

		//! \return A pointer to the WidgetPropertySet owned by this Manager.
		WidgetPropertySet* propertySet() const { return m_propSet; }

		/*! \return true if one of the insert buttons was pressed and the forms
		 are ready to create a widget. */
		bool isInserting() const { return m_inserting; }

		/*! \return The name of the class being inserted, corresponding
		 to the menu item or the toolbar button clicked. */
		QCString selectedClass() const { return m_selectedClass; }

		/*! Sets the point where the pasted widget should be moved to. */
		void setInsertPoint(const QPoint &p);

		//! \return If we are creating a Connection by drag-and-drop or not.
		bool isCreatingConnection() { return m_drawingSlot; }

		//! \return the \ref Connection being created.
		Connection* createdConnection() { return m_connection; }

		/*! Resets the Connection being created. We stay in Connection creation mode,
		  but we start a new connection (when the user clicks
		 outside of signals/slots menu). */
		void resetCreatedConnection();

		//! Creates and display a menu with all the signals of widget \a w.
		void createSignalMenu(QWidget *w);

		//! Creates and display a menu with all the slots of widget \a w.
		void createSlotMenu(QWidget *w);

		//! Emits the signal \ref createFormSlot(). Used by \ref WidgetPropertySet.
		void  emitCreateSlot(const QString &widget, const QString &value)
			{ emit createFormSlot(m_active, widget, value); }

		/*! \return The Form actually active and focused.
		 */
		Form* activeForm() const;

		/*! \return the Form whose toplevel widget is \a w, or 0
		 if there is not or the Form is in preview mode. */
		Form* formForWidget(QWidget *w);

		/*! \return true if \a w is a toplevel widget,
		 ie. it is the main widget of a Form (so it should have a caption ,
		 an icon ...) */
		bool isTopLevel(QWidget *w);

		//! \return A pointer to the KoProperty::Editor we use.
		KoProperty::Editor* propertyEditor() const { return m_editor; }

		/*! Shows a propertybuffer \a buff in a Property Editor.
		 If \a buff is 0, Property Editor will be cleared.
		 If \a forceReload is true, the buffer needs to be reloaded even
		 if it's the same as previous one. */
		virtual void showPropertySet(WidgetPropertySet *list, bool forceReload = false);

		/*! Sets the external property editor pane used by FormDesigner (it may be docked).*/
		void setEditor(KoProperty::Editor *editor);

		/*! Sets the external object tree view used by FormDesigner (it may be docked).
		 This function also connects appropriate signals and slots to ensure
		 sync with the current Form. */
		void setObjectTreeView(ObjectTreeView *treeview);

		/*! Previews the Form \a form using the widget \a w as toplevel container for this Form. */
		void previewForm(Form *form, QWidget *w, Form *toForm=0);

		/*! Adds a existing form w and changes it to a container */
		void importForm(Form *form=0, bool preview=false);

		/*! Deletes the Form \a form and removes it from our list. */
		void deleteForm(Form *form);

		/*! This function creates and displays the context menu corresponding to the widget \a w.
		    The menu item are disabled if necessary, and
		    the widget specific part is added (menu from the factory and buddy selection). */
		void  createContextMenu(QWidget *w, Container *container, bool popupAtCursor = true);

		//! \return If we align widgets to grid or not.
		bool  snapWidgetsToGrid();

		//! @internal used by Container
		int contextMenuKey() const { return m_contextMenuKey; }

		//! @internal
		void emitWidgetSelected( KFormDesigner::Form* form, bool multiple );
		//! @internal
		void emitFormWidgetSelected( KFormDesigner::Form* form );
		//! @internal
		void emitNoFormSelected();

	public slots:
		/*! Deletes the selected widget in active Form and all of its children. */
		void deleteWidget();

		/*! Copies the slected widget and all its children of the active Form using an XML representation. */
		void copyWidget();

		/*! Cuts (ie Copies and deletes) the selected widget and all its children of
		 the active Form using an XML representation. */
		void cutWidget();

		/*! Pastes the XML representation of the copied or cut widget. The widget is
		  pasted when the user clicks the Form to
		  indicate the new position of the widget, or at the position of the contextual menu if there is one. */
		void pasteWidget();

		/*! Selects all toplevel widgets in trhe current form. */
		void selectAll();

		/*! Clears the contents of the selected widget(s) (eg for a line edit or a listview). */
		void clearWidgetContent();

		void undo();
		void redo();

		/*! Displays a dialog where the user can modify the tab order of the active Form,
		 by drag-n-drop or using up/down buttons. */
		void editTabOrder();

		/*! Adjusts the size of the selected widget, ie resize it to its size hint. */
		void adjustWidgetSize();

		/*! Creates a dialog to edit the \ref activeForm() PixmapCollection. */
		void editFormPixmapCollection();

		/*! Creates a dialog to edit the Connection of \ref activeForm(). */
		void editConnections();

		//! Lay out selected widgets using HBox layout (calls \ref CreateLayoutCommand).
		void layoutHBox();

		//! Lay out selected widgets using VBox layout.
		void layoutVBox();

		//! Lay out selected widgets using Grid layout.
		void layoutGrid();

		//! Lay out selected widgets in an horizontal splitter
		void  layoutHSplitter();
		//! Lay out selected widgets in a verticak splitter
		void  layoutVSplitter();

		//! Breaks selected layout(calls \ref BreakLayoutCommand).
		void breakLayout();

		void alignWidgetsToLeft();
		void alignWidgetsToRight();
		void alignWidgetsToTop();
		void alignWidgetsToBottom();
		void alignWidgetsToGrid();

		void adjustSizeToGrid();

		//! Resize all selected widgets to the width of the narrowest widget.
		void adjustWidthToSmall();

		//! Resize all selected widgets to the width of the widest widget.
		void adjustWidthToBig();

		//! Resize all selected widgets to the height of the shortest widget.
		void adjustHeightToSmall();

		//! Resize all selected widgets to the height of the tallest widget.
		void adjustHeightToBig();

		void bringWidgetToFront();
		void sendWidgetToBack();

		/*! This slot is called when the user presses a "Widget" toolbar button
		  or a "Widget" menu item. Prepares all Forms for
		  creation of a new widget (ie changes cursor ...).
		 */
		void insertWidget(const QCString &classname);

		/*! Stops the current widget insertion (ie unset the cursor ...). */
		void stopInsert();

		//! Slot called when the user presses 'Pointer' icon. Switch to Default mode.
		void slotPointerClicked();

		//! Enter the Connection creation mode.
		void startCreatingConnection();

		//! Leave the Connection creation mode.
		void stopCreatingConnection();

		/*! Calls this slot when the window activated changes (eg connect
		 to QWorkspace::windowActivated(QWidget*)). You <b>need</b> to connect
		 to this slot, it will crash otherwise.
		*/
		void windowChanged(QWidget *w);

		//! Used to delayed widgets' deletion (in Container::deleteItem())
		void deleteWidgetLater( QWidget *w );

		/*! For debugging purposes only:
		 shows a text window containing contents of .ui XML definition of the current form. */
		void showFormUICode();

		/*! Executes font dialog and changes it for currently selected widget(s). */
		void changeFont();

	signals:
		/*! This signal is emited as the property buffer switched.
		 If \a forceReload is true, the buffer needs to be reloaded even
		 if it's the same as previous one. */
		void propertySetSwitched(KoProperty::Set *list, bool forceReload = false);

		/*! This signal is emitted when any change is made to the Form \a form,
		 so it will need to be saved. */
		void dirty(KFormDesigner::Form *form, bool isDirty=true);

		/*! Signal emitted when a normal widget is selected inside \a form
		 (ie not form widget). If \a multiple is true,
		 then more than one widget is selected. Use this to update actions state. */
		void widgetSelected(KFormDesigner::Form *form, bool multiple);

		/*! Signal emitted when the form widget is selected inside \a form.
		 Use this to update actions state. */
		void formWidgetSelected(KFormDesigner::Form *form);

		/*! Signal emitted when no form (or a preview form) is selected.
		 Use this to update actions state. */
		void noFormSelected();

		/*! Signal emitted when undo action activation changes.
		 \a text is the full text of the action (including command name). */
		void undoEnabled(bool enabled, const QString &text = QString::null);

		/*! Signal emitted when redo action activation changes.
		 \a text is the full text of the action (including command name). */
		void redoEnabled(bool enabled, const QString &text = QString::null);

		/*! Signal emitted when the user choose a signal in 'Events' menu
		 in context menu, or in 'Events' in property editor.
		 The code editor should then create the slot connected to this signal. */
		void createFormSlot(KFormDesigner::Form *form, const QString &widget, const QString &signal);

		/*! Signal emitted when the Connection creation by drag-and-drop ends.
		 \a connection is the created Connection. You should copy it,
		  because it is deleted just after the signal is emitted. */
		void connectionCreated(KFormDesigner::Form *form, KFormDesigner::Connection &connection);

		/*! Signal emitted when the Connection creation by drag-and-drop is aborted by user. */
		void connectionAborted(KFormDesigner::Form *form);

		/*! Signal emitted when "autoTabStops" is changed. */
		void autoTabStopsSet(KFormDesigner::Form *form, bool set);

		/*! Signal emitted before the form gets finally deleted. \a form is still a valid pointer,
		 but the widgets inside the form are in unknown state. */
		void aboutToDeleteForm(KFormDesigner::Form *form);

		/*! Signal emitted when new form gets created.  */
		void formCreated(KFormDesigner::Form *form);

	protected slots:
		void deleteWidgetLaterTimeout();

		/*! Slot called when a buddy is choosed in the buddy list. Sets the label buddy. */
		void buddyChoosed(int id);

		/*! Slot called when the user chooses an item in signal (or slot) menu.
		 The \ref createdConnection() is updated, and the connection created
		 (for the signal menu). */
		void menuSignalChoosed(int id);

		/*! Slot called when the user changes current style using combbox in toolbar or menu. */
		void slotStyle();

		void slotConnectionCreated(KFormDesigner::Form*, KFormDesigner::Connection&);

		void slotSettingsChanged(int category);

	protected:
		/*! Inits the Form, adds it to m_forms, and conects slots. */
		void initForm(Form *form);

#if 0
		/*! Default implementation just calls FormIO::loadFormFromDom().
		 Change this if you need to handle, eg. custom UI XML tags, as in Kexi's Form Designer. */
		virtual bool loadFormFromDomInternal(Form *form, QWidget *container, QDomDocument &inBuf);

		/*! Default implementation just calls FormIO::saveFormToString().
		 Change this if you need to handle, eg. custom UI XML tags, as in Kexi's Form Designer. */
		virtual bool saveFormToStringInternal(Form *form, QString &dest, int indent = 0);
#endif
		/*! Function called by the "Lay out in..." menu items. It creates a layout from the
		  currently selected widgets (that must have the same parent).
		  Calls \ref CreateLayoutCommand. */
		void createLayout(int layoutType);

		/*! Function called by all other AlignWidgets*() function. Calls \ref AlignWidgetsCommand. */
		void alignWidgets(int type);

		void enableFormActions();
		void disableWidgetActions();
		void emitUndoEnabled(bool enabled, const QString &text);
		void emitRedoEnabled(bool enabled, const QString &text);

	private:
		//! Enum for menu items indexes
		enum { MenuTitle = 200, MenuCopy, MenuCut, MenuPaste, MenuDelete, MenuHBox = 301,
			MenuVBox, MenuGrid, MenuHSplitter, MenuVSplitter, MenuNoBuddy = 501 };

		WidgetPropertySet *m_propSet;
		WidgetLibrary *m_lib;
		QGuardedPtr<KoProperty::Editor>  m_editor;
		QGuardedPtr<ObjectTreeView>  m_treeview;
		// Forms
		QPtrList<Form>		m_forms;
		QPtrList<Form>		m_preview;
		QGuardedPtr<Form>	m_active;

		// Copy/Paste
		QDomDocument		m_domDoc;
		KPopupMenu		*m_popup;
		QPoint			m_insertPoint;
		QGuardedPtr<QWidget>	m_menuWidget;

		// Insertion
		bool			m_inserting;
		QCString			m_selectedClass;

		// Connection stuff
		bool			m_drawingSlot;
		Connection		*m_connection;
		KPopupMenu		*m_sigSlotMenu;

		// Actions
		KActionCollection	*m_collection;
		KToggleAction		*m_pointer, *m_dragConnection, *m_snapToGrid;

		//! Used to delayed widgets deletion
		QTimer m_deleteWidgetLater_timer;
		QPtrList<QWidget> m_deleteWidgetLater_list;

#ifdef KEXI_SHOW_DEBUG_ACTIONS
		KDialogBase *m_uiCodeDialog;
		KTextEdit *m_currentUICodeDialogEditor;
		KTextEdit *m_originalUICodeDialogEditor;
#endif

		int m_options; //!< @see Options enum
		int m_contextMenuKey; //!< Id of context menu key (cached)

		PropertyFactory *m_propFactory;

		friend class PropertyCommand;
		friend class GeometryPropertyCommand;
		friend class CutWidgetCommand;
		friend class Form;
};

}

#endif
