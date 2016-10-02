#ifndef EASYMOTIONHANDLER_H
#define EASYMOTIONHANDLER_H

#include <coreplugin/icore.h>
#include <coreplugin/icontext.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/coreconstants.h>
#include <texteditor/texteditor.h>
#include <extensionsystem/pluginmanager.h>
#include <QListWidget>
#include <QAction>
#include <QMessageBox>
#include <QMainWindow>
#include <QMenu>

#include <QtPlugin>
#include <QObject>
#include <QApplication>
#include <QPlainTextEdit>
#include <QLabel>
#include <QStatusBar>
#include <QDebug>
#include <QPainter>
#include <QString>
#include <QTextBlock>
#include <QChar>
#include <QTextDocument>
#include <QTextBlock>
#include <QPair>
#include <algorithm>

#include "EasyMotionTarget.h"

#define EDITOR(e) ((m_plainEdit != NULL) ? m_plainEdit->e : m_textEdit->e)

namespace EasyMotion {

class EasyMotionHandler : public QObject
{
  Q_OBJECT

public:
  EasyMotionHandler(QObject *parent = 0);
  ~EasyMotionHandler() {}

public slots:
  void easyMotionForCurrentLineTriggered(void);
  void easyMotionForEntireScreenTriggered(void);

private slots:
  void doInstallEventFilter();
  void findFakeVimStatusWidget();

private:
  QLabel* someLabel;

  void installEventFilter();
  void initEasyMotion();
  void resetEasyMotion(void);

  bool isVisualMode() const;

  bool eventFilter(QObject *obj, QEvent *event);
  bool handleKeyPress(QKeyEvent *e);
  bool handlePaintEvent(QPaintEvent *e);

  void setTextPosition(QRect &rect);
  bool isModifierKey(int key);

  bool setEditor(Core::IEditor *e);
  void unsetEditor();

  enum EasyMotionState {
    DefaultState,
    EasyMotionTriggered,
    WaitForInputTargetCode
  };

  Core::IEditor *m_currentEditor;
  QPlainTextEdit *m_plainEdit;
  QTextEdit *m_textEdit;
  QLabel *m_fakeVimStatusWidget;
  EasyMotionState m_state;
  EasyMotion::EasyMotionTarget m_target;
  int m_easyMotionSearchRange;
  enum SearchRange {
    EntireScreen,
    CurrentLine
  };

  template <class Editor>
  void moveToPosition(Editor *editor, int newPos, bool visualMode)
  {
      QTextBlock targetBlock = editor->document()->findBlock(newPos);
      if (!targetBlock.isValid())
          targetBlock = editor->document()->lastBlock();

      bool overwriteMode = editor->overwriteMode();
      TextEditor::TextEditorWidget *baseEditor =
              qobject_cast<TextEditor::TextEditorWidget*>(editor);
      bool visualBlockMode = baseEditor && baseEditor->hasBlockSelection();

      bool selectNextCharacter = (overwriteMode || visualMode) && !visualBlockMode;
      bool keepSelection = visualMode || visualBlockMode;

      QTextCursor textCursor = editor->textCursor();
      textCursor.setPosition(selectNextCharacter ? newPos : newPos + 1,
                             keepSelection ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);

      if (baseEditor)
          baseEditor->setTextCursor(textCursor);
      else
          editor->setTextCursor(textCursor);

      if (visualBlockMode) {
          baseEditor->setBlockSelection(false);
          baseEditor->setBlockSelection(true);
      }
  }
};

} // namespace EasyMotion

#endif // EASYMOTIONHANDLER_H
