#include "EasyMotionHandler.h"
#include <coreplugin/icore.h>

namespace EasyMotion{

EasyMotionHandler::EasyMotionHandler(QObject *parent)
    : QObject(parent)
    , m_currentEditor(NULL)
    , m_plainEdit(NULL)
    , m_textEdit(NULL)
    , m_fakeVimStatusWidget(0)
    , m_state(DefaultState)
    , m_easyMotionSearchRange(-1)
{
    QMetaObject::invokeMethod(this, "findFakeVimStatusWidget", Qt::QueuedConnection);

    someLabel = new QLabel( Core::ICore::mainWindow() );
    someLabel->hide();

}

void EasyMotionHandler::easyMotionForCurrentLineTriggered()
{
    m_easyMotionSearchRange = CurrentLine;
    initEasyMotion();
    QKeyEvent event(QEvent::None, Qt::Key_0, Qt::NoModifier);
    handleKeyPress(&event);
}

void EasyMotionHandler::easyMotionForEntireScreenTriggered()
{
    m_easyMotionSearchRange = EntireScreen;
    initEasyMotion();
}

void EasyMotionHandler::doInstallEventFilter()
{
    if (m_plainEdit || m_textEdit) {
        EDITOR(installEventFilter(this));
        EDITOR(viewport())->installEventFilter(this);
    }
}

void EasyMotionHandler::findFakeVimStatusWidget()
{
    QWidget *statusBar = Core::ICore::statusBar();
    foreach (QWidget *w, statusBar->findChildren<QWidget*>()) {
        if (QLatin1String(w->metaObject()->className()) == QLatin1String("FakeVim::Internal::MiniBuffer")) {
            m_fakeVimStatusWidget = w->findChild<QLabel*>();
            break;
        }
    }
}

void EasyMotionHandler::installEventFilter()
{
    // Postpone installEventFilter() so plugin gets next key event first.
    QMetaObject::invokeMethod(this, "doInstallEventFilter", Qt::QueuedConnection);
}

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

void EasyMotionHandler::resetEasyMotion()
{
    if (setEditor(m_currentEditor)) {
        QWidget *viewport = EDITOR(viewport());
        EDITOR(removeEventFilter(this));
        viewport->removeEventFilter(this);
        unsetEditor();
    }
    m_target.clear();
    m_state = DefaultState;
    m_currentEditor = NULL;
}

bool EasyMotionHandler::isVisualMode() const
{
    if (m_fakeVimStatusWidget)
        return m_fakeVimStatusWidget->text().contains(QLatin1String("VISUAL"));
    return (m_plainEdit || m_textEdit) && EDITOR(textCursor()).hasSelection();
}

bool EasyMotionHandler::eventFilter(QObject *obj, QEvent *event)
{
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

bool EasyMotionHandler::handleKeyPress(QKeyEvent *e)
{
    someLabel->setWindowFlags(Qt::FramelessWindowHint);
    someLabel->show();
    someLabel->setGeometry(500,500,500,500);
    someLabel->setText("I AM SOME TEXT YAY");

    if (e->key() == Qt::Key_Escape) {
        EasyMotionState tmpState = m_state;
        if (tmpState == WaitForInputTargetCode) {
            EDITOR(viewport()->update());
        }
        resetEasyMotion();
        return true;
    }

    else if (m_state == EasyMotionTriggered && !isModifierKey(e->key()))
    {
        QChar target(e->key());
        target = target.toLower();

        if (e->modifiers() == Qt::ShiftModifier)
            target = target.toUpper();

        if (m_plainEdit) {
            if (m_easyMotionSearchRange == EntireScreen) {
                m_target.searchTargetFromScreen(m_plainEdit, target);
            } else {
                m_target.searchTargetFromCurrentLine(m_plainEdit);
            }
        }
        else if (m_textEdit) {
            if (m_easyMotionSearchRange == EntireScreen) {
                m_target.searchTargetFromScreen(m_textEdit, target);
            } else {
                m_target.searchTargetFromCurrentLine(m_textEdit);
            }
        }
        else {
            qDebug() << "EasyMotionHandler::handleKeyPress() => Error: current editor is null";
        }

        if (!m_target.isEmpty()) {
            m_state = WaitForInputTargetCode;
            EDITOR(viewport()->update());
        }
        return true;
    }

    else if (m_state == WaitForInputTargetCode && !isModifierKey(e->key()))
    {
        if (e->key() == Qt::Key_Return) {
            if (e->modifiers() == Qt::ShiftModifier) {
                // Shift + Enter makes EasyMotion show previous
                // group of target positions
                m_target.previousGroup();
            } else {
                // Enter makes EasyMotion show next
                // group of target positions
                m_target.nextGroup();
            }
            EDITOR(viewport()->update());
        } else {
            QChar target(e->key());
            target = target.toLower();
            if (e->modifiers() == Qt::ShiftModifier) target = target.toUpper();
            int newPos = m_target.getTargetPos(target);
            if (newPos >= 0) {
                QPlainTextEdit* plainEdit = m_plainEdit;
                QTextEdit* textEdit = m_textEdit;
                QWidget *viewport = EDITOR(viewport());
                resetEasyMotion();
                if (plainEdit)
                    moveToPosition(plainEdit, newPos, isVisualMode());
                else if (textEdit)
                    moveToPosition(textEdit, newPos, isVisualMode());
                viewport->update();
            }
        }
        return true;
    }
    return false;
}

bool EasyMotionHandler::handlePaintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    if (m_state == WaitForInputTargetCode && !m_target.isEmpty()) {
        QTextCursor tc = EDITOR(textCursor());
        QFontMetrics fm(EDITOR(font()));
        QPainter painter(EDITOR(viewport()));
        QPen pen;
        pen.setColor(QColor(255, 0, 0, 255));
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.setBrush(QBrush(QColor(255, 255, 0, 255)));
        painter.setFont(EDITOR(font()));
        for (int i = m_target.getFirstTargetIndex(); i < m_target.getLastTargetIndex(); ++i) {
            QPair<int, QChar> target = m_target.getTarget(i);
            tc.setPosition(target.first);
            QRect rect = EDITOR(cursorRect(tc));
            int targetCharFontWidth = fm.width(EDITOR(document())->characterAt(target.first));
            if (targetCharFontWidth == 0) targetCharFontWidth = fm.width(QChar(ushort(' ')));
            rect.setWidth(targetCharFontWidth);
            if (rect.intersects(EDITOR(viewport()->rect()))) {
                setTextPosition(rect);
                painter.setPen(Qt::NoPen);
                painter.drawRect(rect);
                painter.setPen(pen);
                int textHeight = rect.bottom() - fm.descent();
                painter.drawText(rect.left(), textHeight, QString(target.second));
            }
        }
        painter.end();
    }
    return false;
}

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

bool EasyMotionHandler::isModifierKey(int key)
{
    return key == Qt::Key_Control
            || key == Qt::Key_Shift
            || key == Qt::Key_Alt
            || key == Qt::Key_Meta;
}

bool EasyMotionHandler::setEditor(Core::IEditor *e)
{
    if (e == NULL) return false;
    QWidget *widget = e->widget();
    m_plainEdit = qobject_cast<QPlainTextEdit *>(widget);
    m_textEdit = qobject_cast<QTextEdit *>(widget);
    return m_plainEdit != NULL || m_textEdit != NULL;
}

void EasyMotionHandler::unsetEditor()
{
    m_plainEdit = NULL;
    m_textEdit = NULL;
}

} // namespace EasyMotion
