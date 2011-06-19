#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

#include <QGraphicsPixmapItem>
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
QT_END_NAMESPACE

class Arrow;
class Token;

class DiagramItem : public QObject, public QGraphicsPolygonItem
{
public:
    enum { Type = UserType + 15 };
    enum DiagramType { Move, Line, Place, Transition, Text };

    DiagramItem(DiagramType diagramType, QMenu *contextMenu,
        QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

    void removeArrow(Arrow *arrow);
    void removeArrows();
    void addToken();
    void addToken(Token *token);
    Token *removeToken(Token *token);
    void removeTokens();
    bool haveSuchArrow(Arrow *a);
    DiagramType diagramType() const
        { return myDiagramType; }
    QPolygonF polygon() const
        { return myPolygon; }
    void addArrow(Arrow *arrow);
    //QPixmap image() const;
    int type() const
        { return Type;}
    int numberOfTokens() const
        { return tokens.length();}
    QList<Arrow *> *arrowsTo();
    QList<Arrow *> *arrowsFrom();
    bool isReadyToFire();

protected:
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    DiagramType myDiagramType;
    QPolygonF myPolygon;
    QMenu *myContextMenu;
    QList<Arrow *> arrows;
    QList<Token *> tokens;
};

#endif
