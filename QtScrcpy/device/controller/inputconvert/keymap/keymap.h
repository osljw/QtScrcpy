#ifndef KEYMAP_H
#define KEYMAP_H
#include <QObject>
#include <QPointF>
#include <QVector>
#include <QRectF>
#include <QJsonObject>
#include <QDebug>
#include <QMetaEnum>

class KeyMap : public QObject
{    
    Q_OBJECT
public:
    enum KeyMapType {
        KMT_INVALID = -1,
        KMT_CLICK = 0,
        KMT_CLICK_TWICE,
        KMT_STEER_WHEEL,
    };    
    Q_ENUM(KeyMapType)

    struct KeyNode {
        int key = Qt::Key_unknown;
        QPointF pos = QPointF(0, 0);
    };

    struct KeyMapNode {
        KeyMapType type = KMT_INVALID;
        union {
            struct {
                KeyNode keyNode;
                bool switchMap = false;
            } click;
            struct {
                KeyNode keyNode;
            } clickTwice;
            struct {
                // 方向盘矩形中心位置                
                QPointF centerPos = {0.0f, 0.0f};

                // 方向盘矩形四个方向偏移量                
                float leftOffset = 0.0f;
                float rightOffset = 0.0f;
                float upOffset = 0.0f;
                float downOffset = 0.0f;

                // 方向盘矩形四个方向按键                
                int leftKey = Qt::Key_unknown;
                int rightKey = Qt::Key_unknown;
                int upKey = Qt::Key_unknown;
                int downKey = Qt::Key_unknown;

                // 辅助变量
                // 方向键的按下状态
                bool leftKeyPressed = false;
                bool rightKeyPressed = false;
                bool upKeyPressed = false;
                bool downKeyPressed = false;
                // 按下方向键的数量
                int pressKeysNum = 0;
                // 第一次按下的键
                int firstPressKey = 0;
            } steerWheel;
        };
        KeyMapNode() {}
        ~KeyMapNode() {}
        QString toString() {
            switch (type) {
            case KMT_STEER_WHEEL:
                 return "KMT_STEER_WHEEL";
            case KMT_CLICK:
                 return "KMT_CLICK";
            default:
                return "KMT_OTHER";

            }
        }

        QJsonObject kmtClickSerialization() {
            QJsonObject output;
            output["type"] = "KMT_CLICK";
            QString key_name = metaEnumKey.valueToKey(click.keyNode.key);
            if (key_name == "") key_name = metaEnumMouseButtons.valueToKey(click.keyNode.key);
            output["key"] = key_name;

            QJsonObject posObject;
            posObject["x"] = click.keyNode.pos.x();
            posObject["y"] = click.keyNode.pos.y();
            output["pos"] = posObject;

            output["switchMap"] = click.switchMap;
            return output;
        }

        QJsonObject kmtSteerWheelSerialization() {
            QJsonObject output;
            output["type"] = "KMT_STEER_WHEEL";
            output["leftOffset"] = steerWheel.leftOffset;
            output["rightOffset"] = steerWheel.rightOffset;
            output["upOffset"] = steerWheel.upOffset;
            output["downOffset"] = steerWheel.downOffset;

            output["leftKey"] = metaEnumKey.valueToKey(steerWheel.leftKey);
            output["rightKey"] = metaEnumKey.valueToKey(steerWheel.rightKey);
            output["upKey"] = metaEnumKey.valueToKey(steerWheel.upKey);
            output["downKey"] = metaEnumKey.valueToKey(steerWheel.downKey);

            QJsonObject posObject;
            posObject["x"] = steerWheel.centerPos.x();
            posObject["y"] = steerWheel.centerPos.y();
            output["centerPos"] = posObject;
            return output;
        }

        QJsonObject toQJsonObject () {
            QJsonObject output;
            switch(type) {
                case KMT_CLICK:
                    return kmtClickSerialization();
                case KMT_STEER_WHEEL:
                    return kmtSteerWheelSerialization();
            }
            return output;
        }
    private:
        QMetaEnum metaEnumKey = QMetaEnum::fromType<Qt::Key>();
        QMetaEnum metaEnumMouseButtons = QMetaEnum::fromType<Qt::MouseButtons>();
    };

    struct MouseMoveMap {
        QPointF startPos = {0.0f, 0.0f};
        int speedRatio = 1;
        QJsonObject toQJsonObject() {
            QJsonObject output;

            QJsonObject startPosObject;
            startPosObject["x"] = startPos.x();
            startPosObject["y"] = startPos.y();
            output["startPos"] = startPosObject;

            output["speedRatio"] = speedRatio;
            return output;
        }
    };

    KeyMap(QObject *parent = Q_NULLPTR);
    virtual ~KeyMap();

    void loadKeyMap(const QString &json);
    void saveKeyMap(const QString &ouput);
    KeyMap::KeyMapNode& getKeyMapNode(int key);
    int getSwitchKey();
    MouseMoveMap getMouseMoveMap();
    bool enableMouseMoveMap();

    static const QString& getKeyMapPath();

//private:
public:
    QVector<KeyMapNode> m_keyMapNodes;
    KeyMapNode m_invalidNode;
    int m_switchKey = Qt::Key_QuoteLeft;
    MouseMoveMap m_mouseMoveMap;
    static QString s_keyMapPath;
};

#endif // KEYMAP_H
