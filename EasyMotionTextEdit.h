#ifndef EASYMOTIONTEXTEDIT_H
#define EASYMOTIONTEXTEDIT_H

#include <QLineEdit>
#include <QTimer>

class EasyMotionTextEdit : public QLineEdit
{
    Q_OBJECT

public:
    EasyMotionTextEdit(QWidget* parent, int width = 130, int height = 30);

    void showAt(QPoint pos);
    bool isVisible();
    void hide();

signals:
    void numberKeyPressed(QChar num);
    void nonNumberKeypressed();
    void enterPressed();
    void cancel();

protected:
    bool eventFilter(QObject *, QEvent *);

private slots:
    void emitNonNumberKeypressed();

private:
    int m_width;
    int m_height;
    bool m_visible;

    QTimer m_keyPressedTimer;

};

#endif // EASYMOTIONTEXTEDIT_H
