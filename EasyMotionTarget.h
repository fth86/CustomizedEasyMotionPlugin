#ifndef EASYMOTIONTARGET_H
#define EASYMOTIONTARGET_H

#include <boost/foreach.hpp>

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

namespace EasyMotion{

class EasyMotionTarget : public QObject
{
  Q_OBJECT
public:
  EasyMotionTarget(void);

  // ------------------------------------------------------------------------------------
  template <class QEditor>
  void searchTargetFromScreen(QEditor *editor, const QString& target)
  {
    m_targetPos.clear();

    if (editor == NULL) {
      return;
    }

    QTextDocument *doc = editor->document();
    QPair<int, int> visibleRange = getFirstAndLastVisiblePosition(editor);
    int  firstPos = visibleRange.first;
    int lastPos = visibleRange.second;
    //  bool notCaseSensative = target.category() != QChar::Letter_Uppercase;

    // -- word matching  ------------------
    QTextCursor previousResult = doc->find(target, firstPos);
    if(previousResult.isNull())
        return;

    m_targetPos << previousResult.anchor() - 1;

    bool searchInProgress = true;
    while( searchInProgress ){
        QTextCursor result = doc->find(target, previousResult);
        if( result.isNull() || result.position() > lastPos)
            searchInProgress=false;
        else{
            m_targetPos << result.anchor() - 1;
            previousResult = result;
        }
    }

  }

  // -----------------------------------------------------------------------------------
  template <class Editor>
  QPair<int, int> getFirstAndLastVisiblePosition(Editor *editor)
  {
    QTextCursor cursor = editor->textCursor();
    QTextDocument *doc = editor->document();
    int currentLine = doc->findBlock(cursor.position()).blockNumber();
    int cursorHeight = editor->cursorRect().height();
    int lineCountToFirstVisibleLine = editor->cursorRect().top() / cursorHeight;
    int firstVisibleLineNum = currentLine - lineCountToFirstVisibleLine;
    if (firstVisibleLineNum < 0) {
      firstVisibleLineNum = 0;
    }
    int maxLineNumOnScreen = (editor->viewport()->height() / cursorHeight);
    if (maxLineNumOnScreen < 1) {
      maxLineNumOnScreen = 1;
    }
    int firstPos = doc->findBlockByNumber(firstVisibleLineNum).position();
    int lastVisibleLineNum = firstVisibleLineNum + maxLineNumOnScreen - 1;
    QTextBlock lastVisibleTextBlock = doc->findBlockByNumber(lastVisibleLineNum);
    if (!lastVisibleTextBlock.isValid()) {
      lastVisibleTextBlock = doc->lastBlock();
    }
    int lastPos = lastVisibleTextBlock.position() + lastVisibleTextBlock.length() - 1;
    return QPair<int, int>(firstPos, lastPos);
  }


  int size() const { return m_targetPos.size();  }
  bool isEmpty() const {  return m_targetPos.size() == 0;  }
  void clear();
  int getFirstTargetIndex(void) const;
  int getLastTargetIndex(void) const;
  QPair<int, QString> getTarget(int i) const;
  QChar getTargetChar(void) const { return m_targetChar; }
  int getTargetPos(const int code) const;

private:
  int parseCode(const QChar &c) const;

  enum {
    LowerLetterCaseStart = 0,
    DigitStart = 26,
    UpperLetterCaseStart = 36,
    GroupSize = 62
  };
  int m_currentGroup;
  QChar m_targetChar;
  QVector<int> m_targetPos;
  QVector<QChar> m_code;
};

} // namespace EasyMotion

#endif // EASYMOTIONTARGET_H
