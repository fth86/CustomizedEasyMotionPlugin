#include "EasyMotionTextEdit.h"

#include <QEvent>
#include <QKeyEvent>

EasyMotionTextEdit::EasyMotionTextEdit(QWidget* parent, int width, int height)
    : QLineEdit(parent)
    , m_width(width)
    , m_height(height)
    , m_visible(false)
{
//    setFocusPolicy(Qt::NoFocus);
//    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowFlags(Qt::Tool);

    this->installEventFilter(this);

    m_keyPressedTimer.setInterval(10);
    connect(&m_keyPressedTimer, SIGNAL(timeout()), this, SLOT(emitNonNumberKeypressed()));

}

void EasyMotionTextEdit::showAt(QPoint pos)
{
    this->setGeometry(pos.x()+50, pos.y()-35, m_width, m_height);
    this->show();
    this->setFocus();

    m_visible = true;
}


void EasyMotionTextEdit::hide()
{
    m_visible = false;
    QLineEdit::hide();
}


bool EasyMotionTextEdit::eventFilter(QObject* watched, QEvent* event)
{

    bool handled;
    handled = QLineEdit::eventFilter(watched,event);

//    QTextEdit *edit = qobject_cast<QTextEdit *>(watched);
    if ( event->type() == QEvent::KeyPress )
    {
        int key = ((QKeyEvent*) event)->key();
        if ( key == Qt::Key_0 ||
             key == Qt::Key_1 ||
             key == Qt::Key_2 ||
             key == Qt::Key_3 ||
             key == Qt::Key_4 ||
             key == Qt::Key_5 ||
             key == Qt::Key_6 ||
             key == Qt::Key_7 ||
             key == Qt::Key_8 ||
             key == Qt::Key_9)  {

            emit numberKeyPressed(QChar(key));
            return true;
        }

        else if(key == Qt::Key_Escape){
            clear();
            emit cancel();
            return true;
        }

        else if(key == Qt::Key_Return){
            clear();
            emit enterPressed();
            return true;
        }

        else{
            m_keyPressedTimer.start();
            return handled;
        }
    }
    else if( event->type() == QEvent::FocusOut ){
        emit cancel();
    }
    return handled;
}

void EasyMotionTextEdit::emitNonNumberKeypressed()
{
    m_keyPressedTimer.stop();
    emit nonNumberKeypressed();
}
