#include "steerform.h"
#include "ui_steerform.h"
#include "keymapform.h"

#include <QPainter>
#include <QMouseEvent>


SteerForm::SteerForm(QWidget *parent, KeyMap::KeyMapNode *keyMap, bool read_state):
    QWidget(parent),
    //ui(new Ui::SteerForm),
    m_parent(parent),
    m_keyMapPointer(keyMap),
    m_read_state(read_state)
{
    //ui->setupUi(this);
    updateData();
    //this->resize(300,300);
}

SteerForm::~SteerForm()
{
    //delete ui;
}


QString SteerForm::parse_key(int key)
{
    QString text = "";

    QMetaEnum metaEnumKey = QMetaEnum::fromType<Qt::Key>();
    QString key_name = metaEnumKey.valueToKey(key);
    //qDebug() << "key_name:" << key_name << "code:" << data.click.keyNode.key;

    if (key_name == "") {
         QMetaEnum metaEnumMouseButtons = QMetaEnum::fromType<Qt::MouseButtons>();
         key_name = metaEnumMouseButtons.valueToKey(key);
         //qDebug() << "key_name:" << key_name << "code:" << data.click.keyNode.key;
     text = key_name;
    } else {
        text = key_name.split('_')[1];
    }

    return text;
}

void SteerForm::updateData()
{
    m_parentSize = m_parent->size();
    KeyMap::KeyMapNode& data = *m_keyMapPointer;

    QPointF relativePos = data.steerWheel.centerPos;
    int x, y;
    x = relativePos.x() * m_parentSize.width();
    y = relativePos.y() * m_parentSize.height();

    // 方向盘矩形四个方向按键
    m_leftKey = parse_key(data.steerWheel.leftKey);
    m_rightKey = parse_key(data.steerWheel.rightKey);
    m_upKey = parse_key(data.steerWheel.upKey);
    m_downKey = parse_key(data.steerWheel.downKey);

    int leftOffset = data.steerWheel.leftOffset * m_parentSize.width();
    int rightOffset = data.steerWheel.rightOffset * m_parentSize.width();
    int upOffset = data.steerWheel.upOffset * m_parentSize.height();
    int downOffset = data.steerWheel.downOffset * m_parentSize.height();

    this->resize(leftOffset+rightOffset+20, upOffset+downOffset+20);

    int xCenter = this->width()/2;
    int yCenter = this->height()/2;
    m_leftKeyPos = QPoint(xCenter - leftOffset, yCenter);
    m_rightKeyPos = QPoint(xCenter + rightOffset, yCenter);
    m_upKeyPos = QPoint(xCenter, yCenter - upOffset);
    m_downKeyPos = QPoint(xCenter, yCenter + downOffset);

    x -= this->pos().x() + this->width()/2;
    y -= this->pos().y() + this->height()/2;
    this->move(x, y);
}



void SteerForm::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    painter.setPen(QColor(0, 160, 230));
    if(m_edit_state) {
        painter.setBrush(Qt::red);
    } else {
        painter.setBrush(Qt::white);
    }
    painter.setOpacity(0.6);
    painter.drawEllipse(rect());

//    qDebug() << "left" << m_leftKeyPos;
//    qDebug() << "right:" << m_rightKeyPos;
//    qDebug() << "up" << m_upKeyPos;
//    qDebug() << "down" << m_downKeyPos;

    QPoint t(10, 10);
    QRect m_leftKeyRect(m_leftKeyPos - t, m_leftKeyPos + t);
    QRect m_rightKeyRect(m_rightKeyPos - t, m_rightKeyPos + t);
    QRect m_upKeyRect(m_upKeyPos - t, m_upKeyPos + t);
    QRect m_downKeyRect(m_downKeyPos - t, m_downKeyPos + t);

    QFont font;
    font.setFamily("Microsoft YaHei");
    font.setPointSize(10);
    painter.setFont(font);
    painter.setPen(Qt::black);
    //painter.drawText(10, 10, m_leftKey);
    painter.drawText(m_leftKeyRect, Qt::AlignCenter, m_leftKey);
    painter.drawText(m_rightKeyRect, Qt::AlignCenter, m_rightKey);
    painter.drawText(m_upKeyRect, Qt::AlignCenter, m_upKey);
    painter.drawText(m_downKeyRect, Qt::AlignCenter, m_downKey);
}


void SteerForm::mousePressEvent(QMouseEvent *e)
{
    if(m_read_state) {
        e->ignore();
        return;
    }
    m_offset = e->pos();
}

void SteerForm::mouseMoveEvent(QMouseEvent *e)
{
    if(m_read_state) {
        e->ignore();
        return;
    }
    if(e->buttons() & Qt::LeftButton) {
        QPoint distance = e->pos() - m_offset;
        this->move(this->pos() + distance);

        QPoint offset(this->width()/2, this->height()/2);
        QPoint p = this->pos() + offset;
        m_keyMapPointer->steerWheel.centerPos.setX(p.x() * 1.0 / m_parentSize.width());
        m_keyMapPointer->steerWheel.centerPos.setY(p.y() * 1.0 / m_parentSize.height());
    }
}

void SteerForm::mouseReleaseEvent(QMouseEvent *e)
{
    if(m_read_state) {
        e->ignore();
        return;
    }
}

void SteerForm::enterEvent(QEvent *event)
{
    if(m_read_state) {
        event->ignore();
        return;
    }
    m_edit_state = !m_edit_state;
    repaint();
}

void SteerForm::leaveEvent(QEvent *event)
{
    if(m_read_state) {
        event->ignore();
        return;
    }
    m_edit_state = !m_edit_state;
    repaint();
}

