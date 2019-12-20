#ifndef STEERFORM_H
#define STEERFORM_H

#include "keymap.h"

#include <QWidget>

namespace Ui {
class SteerForm;
}

class SteerForm : public QWidget
{
    Q_OBJECT
public:
    //explicit Steer(QWidget *parent = nullptr);
    explicit SteerForm(QWidget *parent, KeyMap::KeyMapNode* keyMap, bool read_state);
    ~SteerForm();

protected:
    void mousePressEvent(QMouseEvent * e) override;
    void mouseMoveEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void paintEvent(QPaintEvent* e) override;
    void updateData();
    QString parse_key(int key);


signals:

public slots:

private:
    KeyMap::KeyMapNode* m_keyMapPointer;
    QString m_leftKey;
    QString m_rightKey;
    QString m_upKey;
    QString m_downKey;

    QPoint m_leftKeyPos;
    QPoint m_rightKeyPos;
    QPoint m_upKeyPos;
    QPoint m_downKeyPos;

    bool m_read_state;
    bool m_edit_state = false;

    QSize m_parentSize;
    QPointF _initial;
    QPointF _current;
    QPoint m_offset;

private:
    Ui::SteerForm *ui;
    QWidget* m_parent;
};

#endif // STEERFORM_H
