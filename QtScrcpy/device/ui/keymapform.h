#ifndef KEYMAPFORM_H
#define KEYMAPFORM_H

#include <QWidget>

#include "keymap.h"

namespace Ui {
class KeyMapForm;
}

class KeyMapForm : public QWidget
{
    Q_OBJECT

public:
    explicit KeyMapForm(QWidget *parent, KeyMap::KeyMapNode* keyMap, bool read_state);
    ~KeyMapForm();

    void updateData();

//private slots:
//    void on_keyEdit_editingFinished();
//    //void on_keyEdit_textEdited(const QString &arg1);

protected:
    void mousePressEvent(QMouseEvent * e) override;
    void mouseMoveEvent(QMouseEvent * e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void paintEvent(QPaintEvent* e) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::KeyMapForm *ui;
    QWidget* m_parent;
    KeyMap::KeyMapNode* m_keyMapPointer;
    QString m_text;
    bool m_read_state;
    bool m_edit_state = false;

    QSize m_parentSize;
    QPointF _initial;
    QPointF _current;
    QPoint m_offset;
};

#endif // KEYMAPFORM_H
