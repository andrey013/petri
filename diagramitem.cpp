#include <QtGui>
#include "diagramitem.h"
#include "diagramtoken.h"
#include "arrow.h"


DiagramItem::DiagramItem(DiagramType diagramType, QMenu *contextMenu,
             QGraphicsItem *parent, QGraphicsScene *scene)
    : QGraphicsPolygonItem(parent, scene)
{
    myDiagramType = diagramType;
    myContextMenu = contextMenu;

    QPainterPath path;
    switch (myDiagramType) {
        case Place:
            path.addEllipse(-12.5,-12.5,25,25);
            myPolygon = path.toFillPolygon();
            break;
        case Transition:
            myPolygon << QPointF(-7, -15) << QPointF(7, -15)
                      << QPointF(7, 15) << QPointF(-7, 15)
                      << QPointF(-7, -15);
            break;
    default:
        ;
    }
    setPolygon(myPolygon);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

void DiagramItem::removeArrow(Arrow *arrow)
{
    int index = arrows.indexOf(arrow);

    if (index != -1) arrows.removeAt(index);
}

void DiagramItem::removeArrows()
{
    foreach (Arrow *arrow, arrows) {
        arrow->startItem()->removeArrow(arrow);
        arrow->endItem()->removeArrow(arrow);
        scene()->removeItem(arrow);
        delete arrow;
    }
}

void DiagramItem::addToken()
{
    QRadialGradient radialGrad(QPointF(0, 0), 4);
    radialGrad.setColorAt(0, Qt::red);
    radialGrad.setColorAt(1, Qt::darkRed);
    Token *t = new Token(this);
    t->setBrush(radialGrad);
    scene()->addItem(t);
    t->setZValue(1000);
    tokens.append(t);
    t->animation->start();
}

void DiagramItem::addToken(Token *token)
{
    //scene()->addItem(token);
    token->setZValue(1000);
    tokens.append(token);
}

Token* DiagramItem::removeToken(Token *token)
{
    if (token == 0){
        if (tokens.length()){
            token = tokens.first();
            tokens.removeFirst();
        }
        return token;
    }
    int index = tokens.indexOf(token);
    if (index != -1) tokens.removeAt(index);
    return token;
}

void DiagramItem::removeTokens()
{
    foreach (Token *token, tokens) {
        removeToken(token);
        scene()->removeItem(token);
        delete token;
    }
}

bool DiagramItem::haveSuchArrow(Arrow *a)
{
    foreach (Arrow *arrow, arrows)
        if(arrow->endItem() == a->endItem())
            return true;
    return false;
}

void DiagramItem::addArrow(Arrow *arrow)
{
    arrows.append(arrow);
}

/*QPixmap DiagramItem::image() const
{
    QPixmap pixmap(25, 25);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 0));
    painter.translate(12.5, 12.5);
    painter.drawPolyline(myPolygon);
    return pixmap;
}
*/
void DiagramItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    scene()->clearSelection();
    setSelected(true);
    myContextMenu->exec(event->screenPos());
}

QVariant DiagramItem::itemChange(GraphicsItemChange change,
                     const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionChange) {
        foreach (Arrow *arrow, arrows) {
            arrow->updatePosition();
        }
    }
    return value;
}


bool DiagramItem::isReadyToFire()
{
    bool haveArrows = false;
    foreach (Arrow *arrow, arrows)
        if(arrow->endItem() == this){
            haveArrows = true;
            if((!arrow->startItem()->numberOfTokens() and !arrow->isInverted()) or
               (arrow->startItem()->numberOfTokens() and arrow->isInverted()))return false;
        }
    foreach (Arrow *arrow, arrows)
        if(arrow->startItem() == this) return haveArrows;
    return false;
}

QList<Arrow *> *DiagramItem::arrowsTo(){
    QList<Arrow *> *a = new QList<Arrow *>;
    foreach (Arrow *arrow, arrows)
        if(arrow->endItem() == this){
            a->append(arrow);
        }
    return a;
}

QList<Arrow *> *DiagramItem::arrowsFrom(){
    QList<Arrow *> *a = new QList<Arrow *>;
    foreach (Arrow *arrow, arrows)
        if(arrow->startItem() == this){
            a->append(arrow);
        }
    return a;
}
