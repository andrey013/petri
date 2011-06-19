#ifndef ARROW_H
#define ARROW_H

#include <QGraphicsLineItem>

QT_BEGIN_NAMESPACE
class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QGraphicsScene;
class QRectF;
class QGraphicsSceneMouseEvent;
class QPainterPath;
QT_END_NAMESPACE

class DiagramItem;

class Arrow : public QGraphicsLineItem
{
public:
    enum { Type = UserType + 4 };

    Arrow(DiagramItem *startItem, DiagramItem *endItem, QMenu *contextMenu,
      QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

    int type() const
        { return Type; }
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void setColor(const QColor &color)
        { myColor = color; }
    void setBendPoint(qreal X, qreal Y)
        {
            bendPoint.setX(X);
            bendPoint.setY(Y);
        }
    void invert();
    bool isInverted()
        { return inverted ;}
    DiagramItem *startItem() const
        { return myStartItem; }
    DiagramItem *endItem() const
        { return myEndItem; }
    QPointF bendPoint;

public slots:
    void updatePosition();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget = 0);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    bool inverted;
    DiagramItem *myStartItem;
    DiagramItem *myEndItem;
    QMenu *myContextMenu;
    QColor myColor;
    QPolygonF arrowHead;

};

#endif
