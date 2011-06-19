#include <QGraphicsScene>

#include "diagramtoken.h"
#include "diagramitem.h"

Token::Token(DiagramItem *place, QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsPolygonItem(parent, scene)
{
    QPainterPath path;
    path.addEllipse(-4,-4,8,8);
    myPolygon = path.toFillPolygon();
    setPolygon(myPolygon);
    myPlace = place;
    destroy = false;
    animation = new QPropertyAnimation(this, "pos");
    animation->setDuration(10);
    //animation->setEasingCurve(QEasingCurve::InOutQuad);
    animation->setStartValue(place->pos());
    animation->setEndValue(place->pos());
    connect(animation, SIGNAL(stateChanged(QAbstractAnimation::State,QAbstractAnimation::State)),
            this, SLOT(startAnimation(QAbstractAnimation::State,QAbstractAnimation::State)));
    //animation->start();
}


void Token::startAnimation ( QAbstractAnimation::State newState,
                      QAbstractAnimation::State oldState )
{
    if(newState==QAbstractAnimation::Stopped){
        if (destroy){
            scene()->removeItem(this);
            delete this;
            return;
        }
        animation->setEasingCurve(QEasingCurve::InOutQuad);
        animation->setDuration(800+qrand()%201);
        animation->setStartValue(this->pos());
        animation->setKeyValueAt(0.5, myPlace->pos() +
                                 QPointF(qrand() % 11 - 5, qrand() % 11 - 5));
        animation->setEndValue(myPlace->pos() +
                               QPointF(qrand() % 11 - 5, qrand() % 11 - 5));
        animation->start();
    }
}

void Token::destroyOnStop(){
    destroy = true;
}
