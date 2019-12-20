#include <QDebug>
#include <QMetaEnum>

#include "controlmsg.h"
#include "bufferutil.h"


ControlMsg::ControlMsg(ControlMsgType controlMsgType)
    : QScrcpyEvent(Control)
{
    m_data.type = controlMsgType;
}

ControlMsg::~ControlMsg()
{
    if (CMT_SET_CLIPBOARD == m_data.type
            && Q_NULLPTR != m_data.setClipboard.text) {
        delete m_data.setClipboard.text;
        m_data.setClipboard.text = Q_NULLPTR;
    } else if (CMT_INJECT_TEXT == m_data.type
               && Q_NULLPTR != m_data.injectText.text){
        delete m_data.injectText.text;
        m_data.injectText.text = Q_NULLPTR;
    }
}

void ControlMsg::setInjectKeycodeMsgData(AndroidKeyeventAction action, AndroidKeycode keycode, AndroidMetastate metastate)
{
    m_data.injectKeycode.action = action;
    m_data.injectKeycode.keycode = keycode;
    m_data.injectKeycode.metastate = metastate;
}

void ControlMsg::setInjectTextMsgData(QString& text)
{
    // write length (2 byte) + string (non nul-terminated)
    if (CONTROL_MSG_TEXT_MAX_LENGTH < text.length()) {
        // injecting a text takes time, so limit the text length
        text = text.left(CONTROL_MSG_TEXT_MAX_LENGTH);
    }
    QByteArray tmp = text.toUtf8();
    m_data.injectText.text = new char[tmp.length() + 1];
    memcpy(m_data.injectText.text, tmp.data(), tmp.length());
    m_data.injectText.text[tmp.length()] = '\0';
}

void ControlMsg::setInjectMouseMsgData(AndroidMotioneventAction action, AndroidMotioneventButtons buttons, QRect position)
{
    m_data.injectMouse.action = action;
    m_data.injectMouse.buttons = buttons;
    m_data.injectMouse.position = position;
}

void ControlMsg::setInjectTouchMsgData(quint32 id, AndroidMotioneventAction action, QRect position)
{
    m_data.injectTouch.action = action;
    m_data.injectTouch.id = id;
    m_data.injectTouch.position = position;
}

void ControlMsg::setInjectScrollMsgData(QRect position, qint32 hScroll, qint32 vScroll)
{
    m_data.injectScroll.position = position;
    m_data.injectScroll.hScroll = hScroll;
    m_data.injectScroll.vScroll = vScroll;
}

void ControlMsg::setSetClipboardMsgData(QString &text)
{
    if (text.isEmpty()) {
        return;
    }
    if (CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH < text.length()) {
        text = text.left(CONTROL_MSG_CLIPBOARD_TEXT_MAX_LENGTH);
    }

    QByteArray tmp = text.toUtf8();
    m_data.setClipboard.text = new char[tmp.length() + 1];
    memcpy(m_data.setClipboard.text, tmp.data(), tmp.length());
    m_data.setClipboard.text[tmp.length()] = '\0';
}

void ControlMsg::setSetScreenPowerModeData(ControlMsg::ScreenPowerMode mode)
{
    m_data.setScreenPowerMode.mode = mode;
}

void ControlMsg::writePosition(QBuffer &buffer, const QRect& value)
{
    BufferUtil::write16(buffer, value.left());
    BufferUtil::write16(buffer, value.top());
    BufferUtil::write16(buffer, value.width());
    BufferUtil::write16(buffer, value.height());
}

QByteArray ControlMsg::serializeData()
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QBuffer::WriteOnly);
    buffer.putChar(m_data.type);

    switch (m_data.type) {
    case CMT_INJECT_KEYCODE:
        buffer.putChar(m_data.injectKeycode.action);
        BufferUtil::write32(buffer, m_data.injectKeycode.keycode);
        BufferUtil::write32(buffer, m_data.injectKeycode.metastate);
        break;
    case CMT_INJECT_TEXT:
        BufferUtil::write16(buffer, strlen(m_data.injectText.text));
        buffer.write(m_data.injectText.text, strlen(m_data.injectText.text));
        break;
    case CMT_INJECT_MOUSE:
        buffer.putChar(m_data.injectMouse.action);
        BufferUtil::write32(buffer, m_data.injectMouse.buttons);
        writePosition(buffer, m_data.injectMouse.position);
        break;
    case CMT_INJECT_TOUCH:
        buffer.putChar(m_data.injectTouch.id);
        buffer.putChar(m_data.injectTouch.action);
        writePosition(buffer, m_data.injectTouch.position);
        break;
    case CMT_INJECT_SCROLL:
        writePosition(buffer, m_data.injectScroll.position);
        BufferUtil::write32(buffer, m_data.injectScroll.hScroll);
        BufferUtil::write32(buffer, m_data.injectScroll.vScroll);
        break;
    case CMT_SET_CLIPBOARD:
        BufferUtil::write16(buffer, strlen(m_data.setClipboard.text));
        buffer.write(m_data.setClipboard.text, strlen(m_data.setClipboard.text));
        break;
    case CMT_SET_SCREEN_POWER_MODE:
        buffer.putChar(m_data.setScreenPowerMode.mode);
        break;
    case CMT_BACK_OR_SCREEN_ON:
    case CMT_EXPAND_NOTIFICATION_PANEL:
    case CMT_COLLAPSE_NOTIFICATION_PANEL:
    case CMT_GET_CLIPBOARD:
        break;
    default:
        qDebug() << "Unknown event type:" << m_data.type;
        break;
    }
    buffer.close();
    return byteArray;
}

//template<typename QEnum>
//QString QtEnumToString (const QEnum value)
//{
//  return QString(QMetaEnum::fromType<QEnum>().valueToKey(value));
//}



QString ControlMsg::toString()
{
    QString ret;
    if(m_data.type == CMT_INJECT_MOUSE) {
        ret +=QString("mouse action:");
        if(m_data.injectMouse.action == AMOTION_EVENT_ACTION_DOWN) {
            ret += "AMOTION_EVENT_ACTION_DOWN";
        }
        if(m_data.injectMouse.action == AMOTION_EVENT_ACTION_UP) {
            ret += "AMOTION_EVENT_ACTION_UP";
        }
        if(m_data.injectMouse.action == AMOTION_EVENT_ACTION_MOVE) {
            ret += "AMOTION_EVENT_ACTION_MOVE";
        }
        if(m_data.injectMouse.action == AMOTION_EVENT_ACTION_CANCEL) {
            ret += "AMOTION_EVENT_ACTION_CANCEL";
        }


    }

    if(m_data.type == CMT_INJECT_TOUCH) {
        ret +=QString("touch id:") + QString::number(m_data.injectTouch.id);

        ret +=QString("touch action:");
        if(m_data.injectTouch.action == AMOTION_EVENT_ACTION_DOWN) {
            ret += "AMOTION_EVENT_ACTION_DOWN";
        }
        if(m_data.injectTouch.action == AMOTION_EVENT_ACTION_UP) {
            ret += "AMOTION_EVENT_ACTION_UP";
        }
//        if(m_data.injectTouch.action == AMOTION_EVENT_ACTION_MOVE) {
//            ret += "AMOTION_EVENT_ACTION_MOVE";
//        }
        if(m_data.injectTouch.action == AMOTION_EVENT_ACTION_CANCEL) {
            ret += "AMOTION_EVENT_ACTION_CANCEL";
        }
    }

    if(m_data.type == CMT_INJECT_KEYCODE) {
        ret +=QString("keyboard action:");
        if(m_data.injectKeycode.action == AMOTION_EVENT_ACTION_DOWN) {
            ret += "AMOTION_EVENT_ACTION_DOWN";
        }
        if(m_data.injectKeycode.action == AMOTION_EVENT_ACTION_UP) {
            ret += "AMOTION_EVENT_ACTION_UP";
        }
        if(m_data.injectKeycode.action == AMOTION_EVENT_ACTION_MOVE) {
            ret += "AMOTION_EVENT_ACTION_MOVE";
        }
        if(m_data.injectKeycode.action == AMOTION_EVENT_ACTION_CANCEL) {
            ret += "AMOTION_EVENT_ACTION_CANCEL";
        }

    }
    return ret;
}

bool ControlMsg::needPrint()
{
    return true;
    if(m_data.type == CMT_INJECT_TOUCH && m_data.injectTouch.action == AMOTION_EVENT_ACTION_MOVE)
        return false;
    return true;
}
