#include "EasyMotionHandler.h"
#include <coreplugin/icore.h>

namespace EasyMotion{

EasyMotionHandler::EasyMotionHandler(QObject *parent)
    : QObject(parent)
    , m_currentEditor(NULL)
    , m_plainEdit(NULL)
    , m_textEdit(NULL)
    , m_state(DefaultState)
    , m_easyMotionSearchRange(-1)
{
//    QMetaObject::invokeMethod(this, "findFakeVimStatusWidget", Qt::QueuedConnection);

//    m_debugWindow = new QTextEdit( Core::ICore::mainWindow() );
//    m_debugWindow->setWindowFlags(Qt::FramelessWindowHint);
//    m_debugWindow->setGeometry(500,100,700,1000);
//    m_debugWindow->setWindowFlags(Qt::WindowStaysOnTopHint);
//    m_debugWindow->setFocusPolicy(Qt::NoFocus);
//    m_debugWindow->show();


    m_feedbackTextEdit = new EasyMotionTextEdit( Core::ICore::mainWindow() );
    m_feedbackTextEdit->setWindowFlags(Qt::FramelessWindowHint);
    m_feedbackTextEdit->hide();

    connect(m_feedbackTextEdit, SIGNAL(numberKeyPressed(QChar)), this, SLOT(numberKeyPressedSlot(QChar)));
    connect(m_feedbackTextEdit, SIGNAL(nonNumberKeypressed()),   this, SLOT(nonNumberKeypressedSlot()));
    connect(m_feedbackTextEdit, SIGNAL(cancel()),                this, SLOT(resetEasyMotion()));
    connect(m_feedbackTextEdit, SIGNAL(enterPressed()),          this, SLOT(enterPressedSlot()) );
}

// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::initEasyMotion()
{
    resetEasyMotion();
    m_currentEditor = Core::EditorManager::currentEditor();
    if (setEditor(m_currentEditor)) {
        m_state = EasyMotionTriggered;
        installEventFilter();
    } else {
        m_currentEditor = NULL;
    }
}

// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::resetEasyMotion()
{


    if (setEditor(m_currentEditor)) {
        QWidget *viewport = EDITOR(viewport());
        viewport->update();
        EDITOR(removeEventFilter(this));
        viewport->removeEventFilter(this);
        unsetEditor();
    }
    m_target.clear();
    m_state = DefaultState;
    m_currentEditor = NULL;
    m_feedbackTextEdit->clear();
    m_feedbackTextEdit->hide();
}


// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::doInstallEventFilter()
{
    if (m_plainEdit || m_textEdit) {
        EDITOR(installEventFilter(this));
        EDITOR(viewport())->installEventFilter(this);
    }
}


// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::installEventFilter()
{
    // Postpone installEventFilter() so plugin gets next key event first.
    QMetaObject::invokeMethod(this, "doInstallEventFilter", Qt::QueuedConnection);
}

// ---------------------------------------------------------------------------------------------
bool EasyMotionHandler::isVisualMode() const
{
    return (m_plainEdit || m_textEdit) && EDITOR(textCursor()).hasSelection();
}


// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::easyMotionForEntireScreenTriggered()
{
    m_easyMotionSearchRange = EntireScreen;
    initEasyMotion();
}


// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::numberKeyPressedSlot(QChar num)
{
    // currently only numbers 0-9 supported, should be enough, ...
    int newPos = m_target.getTargetPos( QString(num).toInt() ) + 1;
    if (newPos >= 0) {

        if (m_plainEdit){
            QTextCursor textCursor = m_plainEdit->textCursor();
            textCursor.setPosition(newPos);
            m_plainEdit->setTextCursor(textCursor);
        }
        else if (m_textEdit){
            QTextCursor textCursor = m_textEdit->textCursor();
            textCursor.setPosition(newPos);
            m_textEdit->setTextCursor(textCursor);
        }
        resetEasyMotion();
//            moveToPosition(m_textEdit, newPos, isVisualMode());

//        EDITOR(viewport())->update();
    }
}

// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::enterPressedSlot()
{

//    m_debugWindow->append("EnterpressedSlot()");
    if(m_target.size() == 0)
        return;

    int newPos = m_target.getTargetPos(1) + 1;
    if (newPos >= 0) {
        if (m_plainEdit){
            QTextCursor textCursor = m_plainEdit->textCursor();
            textCursor.setPosition(newPos);
            m_plainEdit->setTextCursor(textCursor);
        }
        else if (m_textEdit){
            QTextCursor textCursor = m_textEdit->textCursor();
            textCursor.setPosition(newPos);
            m_textEdit->setTextCursor(textCursor);
        }
        resetEasyMotion();
    }
}

// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::nonNumberKeypressedSlot()
{

    if (m_state == EasyMotionTriggered)
    {

        QString target = m_feedbackTextEdit->displayText();

        if (m_plainEdit) {
            m_target.searchTargetFromScreen(m_plainEdit, target);
//            updateDebugWindow();
        }
        else if (m_textEdit) {
            m_target.searchTargetFromScreen(m_textEdit, target);
//            updateDebugWindow();
        }

        EDITOR(viewport()->update());
    }
}


// ---------------------------------------------------------------------------------------------
bool EasyMotionHandler::eventFilter(QObject *obj, QEvent *event)
{
    // obj :   the qobject this eventFilter is watching (and intercepting calls from)
    //

    QWidget *currentViewport = qobject_cast<QWidget *>(obj);

    if (currentViewport != NULL
            && event->type() == QEvent::Paint)  {
        // Handle the painter event last to prevent
        // the area painted by EasyMotion to be overidden
        currentViewport->removeEventFilter(this);
        QCoreApplication::sendEvent(currentViewport, event);
        currentViewport->installEventFilter(this);
        QPaintEvent *paintEvent = static_cast<QPaintEvent *>(event);
        handlePaintEvent(paintEvent);
        return true;

    } else if (event->type() == QEvent::KeyPress) {
        if (m_plainEdit || m_textEdit) {
            //        QMessageBox::information(Core::ICore::mainWindow(),
            //                                 tr("Action triggered"),
            //                                 tr("key"));
            installEventFilter();
            QKeyEvent *e = static_cast<QKeyEvent *>(event);
            bool keyPressHandled = handleKeyPress(e);
            return keyPressHandled;
        }
    }  else if (event->type() == QEvent::ShortcutOverride) {
        installEventFilter();
        // Handle ESC key press.
        QKeyEvent *e = static_cast<QKeyEvent *>(event);
        if (e->key() == Qt::Key_Escape)
            return handleKeyPress(e);
    }
    return false;
}


// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::updateDebugWindow()
{
    m_debugWindow->clear();

    m_debugWindow->append("searching for:" + m_feedbackTextEdit->displayText());
    m_debugWindow->append(QString("m_target size: %1").arg(m_target.size()));

    m_debugWindow->append("m_target contents: ");
    for (int i = m_target.getFirstTargetIndex(); i < m_target.getLastTargetIndex(); ++i) {
        m_debugWindow->append( QString::number(m_target.getTarget(i).first) + " : " + m_target.getTarget(i).second );
    }

//    QRect geo = m_feedbackTextEdit->geometry();

//    m_debugWindow->setGeometry(geo.right()+50, geo.top(), 500,300);
//    m_debugWindow->show();
}


// ---------------------------------------------------------------------------------------------
bool EasyMotionHandler::handleKeyPress(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape) {
        resetEasyMotion();
        return true;
    }
    else if(m_state == EasyMotionTriggered && !isModifierKey(e->key()) ){

        if (m_plainEdit) {
            QRect geom = m_plainEdit->geometry();
            QPoint pos = m_plainEdit->mapToGlobal( geom.bottomLeft());
            m_feedbackTextEdit->showAt(pos);
        }
        else if (m_textEdit) {
            QRect geom = m_textEdit->geometry();
            QPoint pos = m_textEdit->mapToGlobal( geom.bottomLeft());
            m_feedbackTextEdit->showAt(pos);
        }

        if(e->modifiers() & Qt::ShiftModifier)
            m_feedbackTextEdit->setText( QString( e->key()).toUpper() );
        else
            m_feedbackTextEdit->setText( QString( e->key() ).toLower() );
        m_feedbackTextEdit->end(false);
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------------------------
bool EasyMotionHandler::handlePaintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    if (m_state == EasyMotionTriggered && !m_target.isEmpty() ){
        QTextCursor tc = EDITOR(textCursor());
        QFontMetrics fm(EDITOR(font()));
        QPainter painter(EDITOR(viewport()));
        QPen pen;
        pen.setColor(QColor(230, 20, 20, 255));
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.setBrush(QBrush(QColor(220, 220, 0, 255)));
        painter.setFont(EDITOR(font()));

        for (int i = m_target.getFirstTargetIndex(); i < m_target.getLastTargetIndex(); ++i) {
            QPair<int, QString> target = m_target.getTarget(i);
            tc.setPosition(target.first);
            QRect rect = EDITOR(cursorRect(tc));
            int targetStringFontWidth = fm.width(target.second[0]);
            rect.setWidth(targetStringFontWidth);

            // draw
            if (rect.intersects(EDITOR(viewport()->rect()))) {
                painter.setPen(Qt::NoPen);
                painter.drawRect(rect);
                painter.setPen(pen);
                int textHeight = rect.bottom() - fm.descent();
                painter.drawText(rect.left(), textHeight, target.second);
            }
        }
        painter.end();
    }
    return false;
}

// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::setTextPosition(QRect &rect)
{
    if (m_easyMotionSearchRange == CurrentLine) {
        int textHeightOffset = EDITOR(cursorRect()).height();
        rect.setTop(rect.top() - textHeightOffset);
        rect.setBottom(rect.bottom() - textHeightOffset);
        if (!rect.intersects(EDITOR(viewport()->rect()))) {
            rect.setTop(rect.top() + 2 * textHeightOffset);
            rect.setBottom(rect.bottom() + 2 * textHeightOffset);
        }
    }
}

// ---------------------------------------------------------------------------------------------
bool EasyMotionHandler::isModifierKey(int key)
{
    return key == Qt::Key_Control
            || key == Qt::Key_Shift
            || key == Qt::Key_Alt
            || key == Qt::Key_Meta;
}

// ---------------------------------------------------------------------------------------------
bool EasyMotionHandler::setEditor(Core::IEditor *e)
{
    if (e == NULL) return false;
    QWidget *widget = e->widget();
    m_plainEdit = qobject_cast<QPlainTextEdit *>(widget);
    m_textEdit = qobject_cast<QTextEdit *>(widget);
    return m_plainEdit != NULL || m_textEdit != NULL;
}

// ---------------------------------------------------------------------------------------------
void EasyMotionHandler::unsetEditor()
{
    m_plainEdit = NULL;
    m_textEdit = NULL;
}



} // namespace EasyMotion
