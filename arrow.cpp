#include <QtGui>

#include "arrow.h"
#include "diagramitem.h"
#include <math.h>

const qreal Pi = 3.1415;

Arrow::Arrow(DiagramItem *startItem, DiagramItem *endItem, QMenu *contextMenu,
         QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsLineItem(parent, scene)
{
    myStartItem = startItem;
    myEndItem = endItem;
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    myColor = Qt::black;
    //setPen(QPen(myColor, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    myContextMenu = contextMenu;    
    QLineF line(mapFromItem(myStartItem, 0, 0), mapFromItem(myEndItem, 0, 0));
    setLine(line);
    bendPoint.setX((line.p1().x()+line.p2().x())/2);
    bendPoint.setY((line.p1().y()+line.p2().y())/2);
    inverted = false;
}

QRectF Arrow::boundingRect() const
{
    qreal extra = 200+(pen().width() + 20) / 2.0;

    return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),
                                      line().p2().y() - line().p1().y()))
        .normalized()
        .adjusted(-extra, -extra, extra, extra);
}

QPainterPath Arrow::shape() const
{
    //QPainterPath path = QGraphicsLineItem::shape();
    QPainterPath path;
    //path.moveTo(line().p2());
    //path.cubicTo(bendPoint, bendPoint, line().p1());
    path.addPolygon(arrowHead);
    return path;
}

void Arrow::updatePosition()
{
    QLineF line(mapFromItem(myStartItem, 0, 0), mapFromItem(myEndItem, 0, 0));

    setLine(line);
}

void Arrow::paint(QPainter *painter, const QStyleOptionGraphicsItem *,
          QWidget *)
{
    if (myStartItem->collidesWithItem(myEndItem))
        return;
    if (isSelected()) setPen(QPen(myColor, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    else setPen(QPen(myColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    QPen myPen = pen();

    //myPen.setColor(myColor);
    qreal arrowSize = 15;
    painter->setPen(myPen);

    QLineF centerLine(bendPoint, myEndItem->pos());
    QPolygonF endPolygon = myEndItem->polygon();
    QPointF p1 = endPolygon.first() + myEndItem->pos();
    QPointF p2;
    QPointF intersectPoint;
    QLineF polyLine;
    for (int i = 1; i < endPolygon.count(); ++i) {
        p2 = endPolygon.at(i) + myEndItem->pos();
        polyLine = QLineF(p1, p2);
        QLineF::IntersectType intersectType =
            polyLine.intersect(centerLine, &intersectPoint);
        if (intersectType == QLineF::BoundedIntersection)
            break;
        p1 = p2;
    }

    setLine(QLineF(intersectPoint, myStartItem->pos()));

    double angle = ::acos((bendPoint.x() - line().p1().x()) / sqrt(
                          (line().p1().x() - bendPoint.x()) * (line().p1().x() - bendPoint.x()) +
                          (line().p1().y() - bendPoint.y()) * (line().p1().y() - bendPoint.y()) )
                          );
    if (bendPoint.y() - line().p1().y() >= 0)
        angle = (Pi * 2) - angle;

    QPointF arrowP1 = line().p1() + QPointF(sin(angle + Pi / 2.3) * arrowSize,
                                        cos(angle + Pi / 2.3) * arrowSize);
    QPointF arrowP2 = line().p1() + QPointF(sin(angle + Pi - Pi / 2.3) * arrowSize,
                                        cos(angle + Pi - Pi / 2.3) * arrowSize);

    arrowHead.clear();
    arrowHead << line().p1() << arrowP1 << arrowP2;
    QPainterPath smoothLine;
    smoothLine.moveTo(line().p2());
    smoothLine.cubicTo(bendPoint, bendPoint, line().p1());

    painter->drawPath(smoothLine);

    if(inverted){
        QPainterPath ellipse;
        ellipse.addEllipse((0.5*(arrowP1+arrowP2)/2+1.5*line().p1())/2, arrowSize/4, arrowSize/4);
        arrowHead = ellipse.toFillPolygon();
    }
    painter->setBrush(myColor);
    painter->drawPolygon(arrowHead);
    /*if (isSelected()) {
        painter->setPen(QPen(myColor.lighter(), 0, Qt::DashLine));
        QLineF myLine = line();
        myLine.translate(arrowP1 - myLine.p1());
        myLine.setP2(line().p2());
        painter->drawLine(myLine);
        myLine = line();
        myLine.translate(arrowP2 - myLine.p1());
        myLine.setP2(line().p2());
        painter->drawLine(myLine);
    }*/
}

void Arrow::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    myContextMenu->exec(event->screenPos());
}

void Arrow::invert(){
    if(myEndItem->diagramType() == DiagramItem::Transition)inverted = not inverted;
}
