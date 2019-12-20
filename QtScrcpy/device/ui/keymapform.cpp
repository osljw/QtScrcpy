#include <QtCore>
#include <QMouseEvent>
#include <QtGlobal>
#include <QSizeGrip>
#include <QGridLayout>
#include <QBoxLayout>
#include <QPainter>

#include "keymapform.h"
#include "ui_keymapform.h"
#include "keymap.h"

KeyMapForm::KeyMapForm(QWidget *parent, KeyMap::KeyMapNode* keyMap, bool read_state) :
    QWidget(parent),
    m_parent(parent),
    ui(new Ui::KeyMapForm),
    m_keyMapPointer(keyMap),
    m_read_state(read_state)
{
    ui->setupUi(this);


//    ui->keyEdit->setVisible(false);
//    ui->closeButton->setVisible(false);

//    if (QKeySequence(m_keyMapPointer->click.keyNode.key).toString() == "G") {
////        qDebug() << "kmap:" <<  m_keyMapPointer->toString();
////        qDebug() << "kmap:" <<  m_keyMapPointer->click.keyNode.key;
////        qDebug() << "kmap:" << QKeySequence(m_keyMapPointer->click.keyNode.key).toString();
////        qDebug() << "keyMap pointer addr:" << &m_keyMapPointer;
////        qDebug() << "keyMap addr:" << &*m_keyMapPointer;
//    }
//    ui->keyEdit->setAlignment(Qt::AlignCenter);
//    ui->keyEdit->setFocusPolicy(Qt::StrongFocus);

    this->setFocusPolicy(Qt::StrongFocus);
    updateData();
    if(m_read_state) {
        return;
    }

//    this->setWindowFlag(Qt::SubWindow);
//    QSizeGrip* sizeGrip = new QSizeGrip(this);

//    QGridLayout* layout = new QGridLayout(this);
//    layout->addWidget(ui->closeButton, 0, 1);
//    layout->addWidget(ui->keyEdit, 1, 0);


//    layout->addWidget(sizeGrip, 1, 0, Qt::AlignBottom | Qt::AlignLeft);


//    layout -> setColumnStretch(0, 4);
//    layout -> setColumnStretch(1, 1);
//    layout -> setRowStretch(0, 1);
//    layout -> setRowStretch(1, 4);
//    layout -> setHorizontalSpacing(1);
//    layout -> setVerticalSpacing(1);

//    connect(ui->closeButton, SIGNAL(clicked()), parent, SLOT(close_keymapform()));
    //connect(ui->keyEdit, SIGNAL(editingFinished()), parent, SLOT(updateKeyMapForm()));
}

KeyMapForm::~KeyMapForm()
{
    delete ui;
}

void KeyMapForm::paintEvent(QPaintEvent *event)
{
//    if(!m_read_state) {
//        return;
//    }

    Q_UNUSED(event);

    QPainter painter(this);
    painter.setPen(QColor(0, 160, 230));
    if(m_edit_state) {
        painter.setBrush(Qt::red);
    } else {
        painter.setBrush(Qt::white);
    }
    painter.setOpacity(0.6);
    painter.drawEllipse(rect());


//    float factor = rect().width() / painter.fontMetrics().width(m_text);
//    //if ((factor < 1) || (factor > 1.25)) {
//    QFont f = painter.font();
//    f.setPointSizeF(f.pointSizeF()*factor);
//    painter.setFont(f);
//    //}

    QFont font;
    font.setFamily("Microsoft YaHei");
    font.setPointSize(5);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.drawText(rect(), Qt::AlignCenter, m_text);
}

void KeyMapForm::resizeEvent(QResizeEvent *event)
{
    //updateData();
    //qDebug() << "reisze=====" << m_parent->size();
}

void KeyMapForm::updateData()
{
    m_parentSize = m_parent->size();

    KeyMap::KeyMapNode& data = *m_keyMapPointer;

    QPointF relativePos = data.click.keyNode.pos;
    int x, y;
    x = relativePos.x() * m_parentSize.width();
    y = relativePos.y() * m_parentSize.height();

    QMetaEnum metaEnumKey = QMetaEnum::fromType<Qt::Key>();
    QString key_name = metaEnumKey.valueToKey(data.click.keyNode.key);
    //qDebug() << "key_name:" << key_name << "code:" << data.click.keyNode.key;

    if (key_name == "") {
     QMetaEnum metaEnumMouseButtons = QMetaEnum::fromType<Qt::MouseButtons>();
     key_name = metaEnumMouseButtons.valueToKey(data.click.keyNode.key);
     //qDebug() << "key_name:" << key_name << "code:" << data.click.keyNode.key;
     m_text = key_name;
    } else {
        m_text = key_name.split('_')[1];
    }

    x -= this->pos().x() + this->width()/2;
    y -= this->pos().y() + this->height()/2;
    this->move(x, y);
}


//void KeyMapForm::on_keyEdit_editingFinished()
//{

////     QMetaEnum metaEnumKey = QMetaEnum::fromType<Qt::Key>();
////     QString k = "Key_" + ui->keyEdit->text().toUpper();
////     m_keyMapPointer->click.keyNode.key = (Qt::Key)metaEnumKey.keyToValue(k.toStdString().c_str());

//}


void KeyMapForm::mousePressEvent(QMouseEvent * e)
{
    if(m_read_state) {
        e->ignore();
        return;
    }

     //qDebug() << "mouse global pos:" << e->globalPos();
     m_offset = e->pos();
     //m_mouseStartPos = e->globalPos();

}

void KeyMapForm::mouseMoveEvent(QMouseEvent * e)
{
    if(m_read_state) {
        e->ignore();
        return;
    }

     if(e->buttons() & Qt::LeftButton) {
         QPoint distance = e->pos() - m_offset;
         this->move(this->pos() + distance);

         // 保存在父组件中的相对位置
         QPoint offset(this->width()/2, this->height()/2);
         QPoint p = this->pos() + offset;
         m_keyMapPointer->click.keyNode.pos.setX(p.x() * 1.0 / m_parentSize.width());
         m_keyMapPointer->click.keyNode.pos.setY(p.y() * 1.0 / m_parentSize.height());
     }
}

void KeyMapForm::mouseReleaseEvent(QMouseEvent *e)
{

    if(m_read_state) {
        e->ignore();
        return;
    }
}

void KeyMapForm::keyPressEvent(QKeyEvent *event)
{
    //qDebug() << "keyPressEvent key press:" << event->key();
    if(m_read_state) {
        event->ignore();
        return;
    }

    Qt::Key k = Qt::Key(event->key());
    if(k == Qt::Key_F1 || k == Qt::Key_Escape) {
        event->ignore();
        return;
    }
    qDebug() << "keyPressEvent key press:" << k;

    QMetaEnum metaEnumKey = QMetaEnum::fromType<Qt::Key>();
    QString key_name = metaEnumKey.valueToKey(k);

    if (key_name == "") {
         QMetaEnum metaEnumMouseButtons = QMetaEnum::fromType<Qt::MouseButtons>();
         key_name = metaEnumMouseButtons.valueToKey(k);
         m_text = key_name;
    } else {
        m_text = key_name.split('_')[1];
    }
    repaint();

    //ui->keyEdit->clear();
    //ui->keyEdit->setText(m_text);

    QString ktmp = "Key_" + m_text;

    qDebug() << "save key:" << ktmp;
    m_keyMapPointer->click.keyNode.key = (Qt::Key)metaEnumKey.keyToValue(ktmp.toStdString().c_str());
}

void KeyMapForm::keyReleaseEvent(QKeyEvent *event) {
    //qDebug() << "keyReleaseEvent key press:" << event->key();
    if(m_read_state) {
        event->ignore();
        return;
    }

    Qt::Key k = Qt::Key(event->key());
    if(k == Qt::Key_F1 || k == Qt::Key_Escape) {
        event->ignore();
        return;
    }
    qDebug() << "keyReleaseEvent key press:" << k;

    QMetaEnum metaEnumKey = QMetaEnum::fromType<Qt::Key>();
    QString key_name = metaEnumKey.valueToKey(k);

    if (key_name == "") {
         QMetaEnum metaEnumMouseButtons = QMetaEnum::fromType<Qt::MouseButtons>();
         key_name = metaEnumMouseButtons.valueToKey(k);
         m_text = key_name;
    } else {
        m_text = key_name.split('_')[1];
    }
    //ui->keyEdit->clear();
    // ui->keyEdit->setText(m_text);

    QString ktmp = "Key_" + m_text;
    qDebug() << "save key:" << ktmp;
    m_keyMapPointer->click.keyNode.key = (Qt::Key)metaEnumKey.keyToValue(ktmp.toStdString().c_str());
}

void KeyMapForm::enterEvent(QEvent *event)
{
    if(m_read_state) {
        event->ignore();
        return;
    }

    //qDebug() << Q_FUNC_INFO << this->objectName();
    m_edit_state = !m_edit_state;
    repaint();
}

void KeyMapForm::leaveEvent(QEvent *event)
{
    if(m_read_state) {
        event->ignore();
        return;
    }

    m_edit_state = !m_edit_state;
    repaint();
}


