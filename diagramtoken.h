#ifndef DIAGRAMTOKEN_H
#define DIAGRAMTOKEN_H

#include <QGraphicsPixmapItem>
#include <QPropertyAnimation>
#include <QList>

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsItem;
class QGraphicsScene;
class QTextEdit;
class QGraphicsSceneMouseEvent;
class QMenu;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class QPolygonF;
class DiagramItem;
QT_END_NAMESPACE



class Token : public QObject, public QGraphicsPolygonItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    enum { Type = UserType + 10 };

    Token(DiagramItem *place, QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);
    QPropertyAnimation *animation;
    QPolygonF polygon() const
        { return myPolygon; }
    int type() const
        { return Type;}
    DiagramItem * place() const
        { return myPlace;}
    void destroyOnStop();

private slots:
    void startAnimation ( QAbstractAnimation::State newState,
                          QAbstractAnimation::State oldState );

private:
    bool destroy;
    QPolygonF myPolygon;
    QMenu *myContextMenu;
    DiagramItem *myPlace;
};

#endif // DIAGRAMTOKEN_H
