#include <QDesktopWidget>
#include <QMouseEvent>
#include <QTimer>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QtWidgets/QHBoxLayout>
#include <QMimeData>
#include <QFileInfo>
#include <QMessageBox>
#include <QTextEdit>

#include "videoform.h"
#include "mousetap/mousetap.h"
#include "ui_videoform.h"
#include "iconhelper.h"
#include "toolform.h"
#include "keymapform.h"
#include "steerform.h"
#include "controller.h"
#include "filehandler.h"
extern "C"
{
#include "libavutil/frame.h"
}

VideoForm::VideoForm(bool skin, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::videoForm)
    , m_skin(skin)
{    
    ui->setupUi(this);
    initUI();
    //updateShowSize(size());
    bool vertical = size().height() > size().width();
    if (m_skin) {
        updateStyleSheet(vertical);
    }
    qDebug() << "videoform init size() = " << size();
    updateShowSize(size());
}

VideoForm::~VideoForm()
{   
    delete ui;
}

void VideoForm::initUI()
{
    setAttribute(Qt::WA_DeleteOnClose);
    if (m_skin) {
        QPixmap phone;
        if (phone.load(":/res/phone.png")) {
            m_widthHeightRatio = 1.0f * phone.width() / phone.height();
        }

#ifndef Q_OS_OSX
        // mac下去掉标题栏影响showfullscreen
        // 去掉标题栏
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        // 根据图片构造异形窗口
        setAttribute(Qt::WA_TranslucentBackground);
#endif
    }

    setMouseTracking(true);    
    ui->videoWidget->setMouseTracking(true);
    ui->videoWidget->hide();
    m_videoWidgetOffset = ui->videoWidget->pos();
}

void VideoForm::onGrabCursor(bool grab)
{
#if defined(Q_OS_WIN32) || defined(Q_OS_OSX)
    MouseTap::getInstance()->enableMouseEventTap(ui->videoWidget, grab);
#else
    Q_UNUSED(grab);
#endif
}

void VideoForm::updateRender(const AVFrame *frame)
{
    //qDebug() << "========m_frameSize:" << m_frameSize << ",frame:"<< QSize(frame->width, frame->height);
    //qDebug() << "----- updateRender ------";
    if (ui->videoWidget->isHidden()) {
        if (m_loadingWidget) {
            m_loadingWidget->close();
        }
        ui->videoWidget->show();
    }
    bool sizeChanged = m_frameSize != QSize(frame->width, frame->height);
    updateShowSize(QSize(frame->width, frame->height));
    ui->videoWidget->setFrameSize(QSize(frame->width, frame->height));
    ui->videoWidget->updateTextures(frame->data[0], frame->data[1], frame->data[2],
            frame->linesize[0], frame->linesize[1], frame->linesize[2]);
    if(sizeChanged) {
        qDebug() << "========m_frameSize:" << m_frameSize << ",frame:"<< QSize(frame->width, frame->height);
        showKeyMapForm();
    }
}

void VideoForm::showToolForm(bool show)
{
    if (!m_toolForm) {
        m_toolForm = new ToolForm(this, ToolForm::AP_OUTSIDE_RIGHT);
        m_toolForm->move(pos().x() + geometry().width(), pos().y() + 30);
        //m_toolForm = new ToolForm(this, ToolForm::AP_OUTSIDE_BOTTOM);
        //m_toolForm->move(pos().x(), pos().y() + geometry().height());
    }
    m_toolForm->setVisible(show);
}

//void VideoForm::showMapManager(bool edit_state)
//{
//    if (!m_mapManager) {
//        m_mapManager = new MapManager(this);
//    }
//    m_toolForm->setVisible(show);
//    m_toolForm->
//}


void VideoForm::close_keymapform()
{
    auto t = m_keyWidget.back();
    delete t;
    m_keyWidget.pop_back();


}

//void VideoForm::updateKeyMapForm() {
//    QObject* s = sender();
//    s->
//}

void VideoForm::showKeyMapForm(bool show)
{
    if (!m_controller) {
        return;
    }

    qDebug() << "is_fullscreen:" << isFullScreen();
    qDebug() << "videoForm size:" << this->size();
    qDebug() << "ui videoWidget size" << ui->videoWidget->size();
    QPointer<InputConvertBase> input_convert =  m_controller->getInputConvert();
    if(!m_keyMap) m_keyMap = input_convert->getKeyMap();
    if(!m_keyMap) return;

    for (auto& t : m_keyWidget) {
        t->setVisible(false);
        delete t;
    }
    m_keyWidget.clear();

    KeyMapForm *edit;
    SteerForm *steer;
    for(auto& key : m_keyMap->m_keyMapNodes) {
        //qDebug() << "key type=" << key.type;

        switch(key.type) {
        case KeyMap::KMT_CLICK:
            edit = new KeyMapForm(ui->videoWidget, &key, m_readState);
            edit->show();
            m_keyWidget.push_back(edit);
            break;
        case KeyMap::KMT_STEER_WHEEL:
            steer = new SteerForm(ui->videoWidget, &key, m_readState);
            steer->show();
            m_keyWidget.push_back(steer);
            break;
        default:
            break;
        }

//        if (key.type == KeyMap::KMT_CLICK) {
//            KeyMapForm *edit = new KeyMapForm(ui->videoWidget, &key, size(), m_readState);
//            //edit->updateData(size(), m_readState);
//            edit->show();
//            m_keyWidget.push_back(edit);
//        }
    }
}


void VideoForm::updateStyleSheet(bool vertical)
{
    if (vertical) {
        setStyleSheet(R"(
                 #videoForm {
                     border-image: url(:/image/videoform/phone-v.png) 150px 142px 85px 142px;
                     border-width: 150px 142px 85px 142px;
                 }
                 )");
    } else {
        setStyleSheet(R"(
                 #videoForm {
                     border-image: url(:/image/videoform/phone-h.png) 142px 85px 142px 150px;
                     border-width: 142px 85px 142px 150px;
                 }
                 )");
    }
    layout()->setContentsMargins(getMargins(vertical));
}

QMargins VideoForm::getMargins(bool vertical)
{
    QMargins margins;
    if (vertical) {
        // QMargins(int left, int top, int right, int bottom)
        margins = QMargins(10, 68, 12, 62);
    } else {
        margins = QMargins(68, 12, 62, 10);
    }
    return margins;
}

void VideoForm::updateScreenRatio(const QSize &newSize)
{
    qDebug() << "new_size" << newSize;
    m_widthHeightRatio = 1.0f * qMin(newSize.width(),newSize.height()) / qMax(newSize.width(),newSize.height());
}

void VideoForm::updateShowSize(const QSize &newSize)
{
    if (m_frameSize != newSize) {
        qDebug() << "======= update show size:" << m_frameSize << "," << newSize;
        m_frameSize = newSize;
        bool vertical = newSize.height() > newSize.width();
        QSize showSize = newSize;
        QDesktopWidget* desktop = QApplication::desktop();
        if (desktop) {
            QRect screenRect = desktop->availableGeometry();
            if (vertical) {
                showSize.setHeight(qMin(newSize.height(), screenRect.height() - 200));
                showSize.setWidth(showSize.height() * m_widthHeightRatio);
            } else {
                showSize.setWidth(qMin(newSize.width(), screenRect.width()/2));
                showSize.setHeight(showSize.width() * m_widthHeightRatio);
            }

            if (layout()) {
                QMargins m = getMargins(vertical);
                showSize.setWidth(showSize.width() + m.left() + m.right());
                showSize.setHeight(showSize.height() + m.top() + m.bottom());
            }

            if (isFullScreen()) {
                switchFullScreen();
            }

            // 窗口居中
            move(screenRect.center() - QRect(0, 0, showSize.width(), showSize.height()).center());
        }

        // 减去标题栏高度 (mark:已经没有标题栏了)
        //int titleBarHeight = style()->pixelMetric(QStyle::PM_TitleBarHeight);
        //showSize.setHeight(showSize.height() - titleBarHeight);

        if (showSize != size()) {
            // stylesheet 放在resize之前
            if (m_skin) {
                updateStyleSheet(vertical);
            }
            
#ifdef Q_OS_OSX
            setFixedSize(showSize);
#else
            resize(showSize);
#endif

            //showKeyMapForm();
        }
    }
}

void VideoForm::switchFullScreen()
{
    if (isFullScreen()) {



#ifdef Q_OS_OSX
        //setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        //show();
#endif
        // 先更新stylesheet, 再进行showNormal, 防止resize两次， stle
        if (m_skin) {
            //updateStyleSheet(height() > width());
            updateStyleSheet(m_frameSize.height() > m_frameSize.width());
        }
        showNormal();
        showToolForm(true);
        showKeyMapForm();
#ifdef Q_OS_WIN32
        ::SetThreadExecutionState(ES_CONTINUOUS);
#endif
    } else {
        // 这种临时增加标题栏再全屏的方案会导致收不到mousemove事件，导致setmousetrack失效
        // mac fullscreen must show title bar
#ifdef Q_OS_OSX
        //setWindowFlags(windowFlags() & ~Qt::FramelessWindowHint);
#endif
        showToolForm(false);
        if (m_skin) {
            layout()->setContentsMargins(0, 0, 0, 0);
        }
        showFullScreen();
        showKeyMapForm();

        // 全屏状态禁止电脑休眠、息屏
#ifdef Q_OS_WIN32
        ::SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);
#endif
    }
    //qDebug() << "==== end switchFullScreen";
}

void VideoForm::staysOnTop(bool top)
{
    bool needShow = false;
    if (isVisible()) {
        needShow = true;
    }
    setWindowFlag(Qt::WindowStaysOnTopHint, top);
    if (m_toolForm) {
        m_toolForm->setWindowFlag(Qt::WindowStaysOnTopHint, top);
    }
    if (needShow) {
        show();
    }
}

Controller *VideoForm::getController()
{
    return m_controller;
}

void VideoForm::setFileHandler(FileHandler *fileHandler)
{
    m_fileHandler = fileHandler;
}

void VideoForm::setSerial(const QString &serial)
{
    m_serial = serial;
}

void VideoForm::setController(Controller *controller)
{
    m_controller = controller;
}

void VideoForm::mousePressEvent(QMouseEvent *event)
{
    if(m_readState) {
        qDebug() << "mouse press:" << event->type();
    }

    if(!m_readState) {
        if (event->buttons() & Qt::LeftButton) {
            qDebug() << "new node======";
            KeyMap::KeyMapNode new_node;
            m_keyMap->m_keyMapNodes.push_back(new_node);
            qDebug() << "key map size:" << m_keyMap->m_keyMapNodes.size();
            KeyMap::KeyMapNode* pkey = &*(m_keyMap->m_keyMapNodes.end() - 1);

            QPoint tmp = event->pos() - ui->videoWidget->pos();
            pkey->type = KeyMap::KMT_CLICK;
            pkey->click.keyNode.pos.setX(tmp.x() * 1.0 / ui->videoWidget->width());
            pkey->click.keyNode.pos.setY(tmp.y() * 1.0 / ui->videoWidget->height());

            KeyMapForm *edit = new KeyMapForm(ui->videoWidget, pkey, m_readState);
            edit->show();
            m_keyWidget.push_back(edit);
        }
        return;
    }


    if (ui->videoWidget->geometry().contains(event->pos())) {
        if (!m_controller) {
            return;
        }
        event->setLocalPos(ui->videoWidget->mapFrom(this, event->localPos().toPoint()));
        m_controller->mouseEvent(event, ui->videoWidget->frameSize(), ui->videoWidget->size());
    } else {
        if (event->button() == Qt::LeftButton) {
            m_dragPosition = event->globalPos() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void VideoForm::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_readState) return;

    if (m_dragPosition.isNull()) {
        if (!m_controller) {
            return;
        }
        event->setLocalPos(ui->videoWidget->mapFrom(this, event->localPos().toPoint()));
        // local check
        QPointF local = event->localPos();
        if (local.x() < 0) {
            local.setX(0);
        }
        if (local.x() > ui->videoWidget->width()) {
            local.setX(ui->videoWidget->width());
        }
        if (local.y() < 0) {
            local.setY(0);
        }
        if (local.y() > ui->videoWidget->height()) {
            local.setY(ui->videoWidget->height());
        }
        event->setLocalPos(local);
        m_controller->mouseEvent(event, ui->videoWidget->frameSize(), ui->videoWidget->size());
    } else {
        m_dragPosition = QPoint(0, 0);
    }
}

void VideoForm::mouseMoveEvent(QMouseEvent *event)
{    
    if(!m_readState) return;

    if (ui->videoWidget->geometry().contains(event->pos())) {
        if (!m_controller) {
            return;
        }
        event->setLocalPos(ui->videoWidget->mapFrom(this, event->localPos().toPoint()));
        m_controller->mouseEvent(event, ui->videoWidget->frameSize(), ui->videoWidget->size());
    } else if (!m_dragPosition.isNull()){
        if (event->buttons() & Qt::LeftButton) {
            move(event->globalPos() - m_dragPosition);
            event->accept();
        }
    }
}

void VideoForm::wheelEvent(QWheelEvent *event)
{
    if (ui->videoWidget->geometry().contains(event->pos())) {
        if (!m_controller) {
            return;
        }
        QPointF pos = ui->videoWidget->mapFrom(this, event->pos());
        /*
        QWheelEvent(const QPointF &pos, const QPointF& globalPos, int delta,
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                Qt::Orientation orient = Qt::Vertical);
        */
        QWheelEvent wheelEvent(pos, event->globalPosF(), event->delta(),
                               event->buttons(), event->modifiers(), event->orientation());
        m_controller->wheelEvent(&wheelEvent, ui->videoWidget->frameSize(), ui->videoWidget->size());
    }
}

void VideoForm::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_F1 == event->key()
        && !event->isAutoRepeat()) {
        if (!m_readState) {
            qDebug() << "=====save";
            m_keyMap->saveKeyMap("tmp.json");
        }

        m_readState = !m_readState;
        showKeyMapForm();
    }

    if (Qt::Key_Escape == event->key()
            && !event->isAutoRepeat()
            && isFullScreen()) {
        switchFullScreen();
    }


    if (!m_controller) {
        return;
    }
    if (event->key() == Qt::Key_C && (event->modifiers() & Qt::ControlModifier)) {
        m_controller->requestDeviceClipboard();
    }
    if (event->key() == Qt::Key_V && (event->modifiers() & Qt::ControlModifier)) {
        if (event->modifiers() & Qt::ShiftModifier) {
            m_controller->setDeviceClipboard();
        } else {
            m_controller->clipboardPaste();
        }
        return;
    }

    //qDebug() << "keyPressEvent" << event->isAutoRepeat();
    m_controller->keyEvent(event, ui->videoWidget->frameSize(), ui->videoWidget->size());
}

void VideoForm::keyReleaseEvent(QKeyEvent *event)
{
    if (!m_controller) {
        return;
    }
    //qDebug() << "keyReleaseEvent" << event->isAutoRepeat();
    m_controller->keyEvent(event, ui->videoWidget->frameSize(), ui->videoWidget->size());
}

void VideoForm::paintEvent(QPaintEvent *paint)
{
    Q_UNUSED(paint);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void VideoForm::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    if (!isFullScreen()) {
        showToolForm();
    }
    qDebug() << "showEvent ui->videoWidget size:" << ui->videoWidget->size();
    //showKeyMapForm();
}

void VideoForm::resizeEvent(QResizeEvent *event)
{
    //qDebug() << "videoform resize:" << size();
}

void VideoForm::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void VideoForm::dragMoveEvent(QDragMoveEvent *event)
{
    Q_UNUSED(event);
}

void VideoForm::dragLeaveEvent(QDragLeaveEvent *event)
{
    Q_UNUSED(event);
}

void VideoForm::dropEvent(QDropEvent *event)
{
    if (!m_fileHandler) {
        return;
    }
    const QMimeData* qm = event->mimeData();
    QString file = qm->urls()[0].toLocalFile();
    QFileInfo fileInfo(file);

    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "QtScrcpy", tr("file does not exist"), QMessageBox::Ok);
        return;
    }

    if (fileInfo.isFile() && fileInfo.suffix() == "apk") {
        m_fileHandler->installApkRequest(m_serial, file);
        return;
    }
    m_fileHandler->pushFileRequest(m_serial, file);
}
