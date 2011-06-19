#include <QtGui>
#include <QDomDocument>

#include "diagramscene.h"
#include "diagramtoken.h"
#include "arrow.h"

DiagramScene::DiagramScene(QMenu *itemMenu, QObject *parent)
    : QGraphicsScene(parent)
{
    myItemMenu = itemMenu;
    myMode = MoveItem;
    myItemType = DiagramItem::Place;
    line = 0;
    textItem = 0;
    myItemColor = Qt::white;
    myTextColor = Qt::black;
    myLineColor = Qt::black;
}

void DiagramScene::setLineColor(const QColor &color)
{
    myLineColor = color;
    if (isItemChange(Arrow::Type)) {
        Arrow *item =
            qgraphicsitem_cast<Arrow *>(selectedItems().first());
        item->setColor(myLineColor);
        update();
    }
}

void DiagramScene::setTextColor(const QColor &color)
{
    myTextColor = color;
    if (isItemChange(DiagramTextItem::Type)) {
        DiagramTextItem *item =
            qgraphicsitem_cast<DiagramTextItem *>(selectedItems().first());
        item->setDefaultTextColor(myTextColor);
    }
}

void DiagramScene::setItemColor(const QColor &color)
{
    myItemColor = color;
    if (isItemChange(DiagramItem::Type)) {
        DiagramItem *item =
            qgraphicsitem_cast<DiagramItem *>(selectedItems().first());
        item->setBrush(myItemColor);
    }
}

void DiagramScene::setFont(const QFont &font)
{
    myFont = font;

    if (isItemChange(DiagramTextItem::Type)) {
        QGraphicsTextItem *item =
            qgraphicsitem_cast<DiagramTextItem *>(selectedItems().first());
        if (item)
            item->setFont(myFont);
    }
}

void DiagramScene::setMode(Mode mode)
{
    myMode = mode;
}

void DiagramScene::setItemType(DiagramItem::DiagramType type)
{
    myItemType = type;
}

void DiagramScene::editorLostFocus(DiagramTextItem *item)
{
    QTextCursor cursor = item->textCursor();
    cursor.clearSelection();
    item->setTextCursor(cursor);

    if (item->toPlainText().isEmpty()) {
        removeItem(item);
        item->deleteLater();
    }
}

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (mouseEvent->button() != Qt::LeftButton)
        return;

    DiagramItem *item;
    switch (myMode) {
        case InsertItem:
            item = new DiagramItem(myItemType, myItemMenu);
            item->setBrush(myItemColor);
            addItem(item);
            item->setPos(mouseEvent->scenePos());
            emit itemInserted(item);
            break;
        case InsertLine:
            line = new QGraphicsLineItem(QLineF(mouseEvent->scenePos(),
                                        mouseEvent->scenePos()));
            line->setPen(QPen(myLineColor, 0));
            addItem(line);
            break;
        case InsertText:
            textItem = new DiagramTextItem();
            textItem->setFont(myFont);
            textItem->setTextInteractionFlags(Qt::TextEditorInteraction);
            textItem->setZValue(1000.0);
            connect(textItem, SIGNAL(lostFocus(DiagramTextItem*)),
                    this, SLOT(editorLostFocus(DiagramTextItem*)));
            connect(textItem, SIGNAL(selectedChange(QGraphicsItem*)),
                    this, SIGNAL(itemSelected(QGraphicsItem*)));
            addItem(textItem);
            textItem->setDefaultTextColor(myTextColor);
            textItem->setPos(mouseEvent->scenePos());
            emit textInserted(textItem);
    default:
        ;
    }
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void DiagramScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (myMode == InsertLine && line != 0) {
        QLineF newLine(line->line().p1(), mouseEvent->scenePos());
        line->setLine(newLine);
    } else if (myMode == MoveItem) {
        if(selectedItems().count())
        if(selectedItems().first()->type()==Arrow::Type){
            Arrow *item =
                qgraphicsitem_cast<Arrow *>(selectedItems().first());
            item->setBendPoint(mouseEvent->scenePos().x(), mouseEvent->scenePos().y());
            item->updatePosition();
        }
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    if (line != 0 && myMode == InsertLine) {
        QList<QGraphicsItem *> startItems = items(line->line().p1());
        while(startItems.count() && startItems.first()->type() != DiagramItem::Type){
            startItems.removeFirst();
        }
        QList<QGraphicsItem *> endItems = items(line->line().p2());
        while(endItems.count() && endItems.first()->type() != DiagramItem::Type){
            endItems.removeFirst();
        }
        removeItem(line);
        delete line;

        if (startItems.count() > 0 && endItems.count() > 0)
        if(startItems.first()->type() == DiagramItem::Type &&
          endItems.first()->type() == DiagramItem::Type &&
          startItems.first() != endItems.first()) {
            DiagramItem *startItem =
                qgraphicsitem_cast<DiagramItem *>(startItems.first());
            DiagramItem *endItem =
                qgraphicsitem_cast<DiagramItem *>(endItems.first());
            if(startItem->diagramType()!=endItem->diagramType()){
                Arrow *arrow = new Arrow(startItem, endItem, myItemMenu);
                if(!startItem->haveSuchArrow(arrow)){
                    arrow->setColor(myLineColor);
                    startItem->addArrow(arrow);
                    endItem->addArrow(arrow);
                    arrow->setZValue(-1000);
                    addItem(arrow);
                    arrow->updatePosition();
                }
            }
        }
    }
    line = 0;
    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

bool DiagramScene::isItemChange(int type)
{
    foreach (QGraphicsItem *item, selectedItems()) {
        if (item->type() == type)
            return true;
    }
    return false;
}

void DiagramScene::doPetriStep()
{
    QList<DiagramItem *> readyTransitions;
    QList<Token *> insertedTokens;
    foreach (QGraphicsItem *item, items()) {
        if (item->type() == DiagramItem::Type){
            DiagramItem * transition = qgraphicsitem_cast<DiagramItem *>(item);
            if(transition->diagramType() == DiagramItem::Transition &&
               transition->isReadyToFire()) readyTransitions.append(transition);
        }
    }
    while(readyTransitions.length()){
        int n = qrand() % readyTransitions.length();
        DiagramItem *item = readyTransitions[n];
        if(item->isReadyToFire()){
            QList<Arrow *> *arrowsToList = item->arrowsTo();
            QList<Arrow *> *arrowsFromList = item->arrowsFrom();
            QPointF startPos;
            foreach (Arrow *arrow, *arrowsToList) {
                Token * t = arrow->startItem()->removeToken(0);
                if(!t){
                    startPos = arrow->startItem()->pos();
                    continue;
                }
                t->animation->stop();
                t->animation->setDuration(400);
                t->animation->setEasingCurve(QEasingCurve::InOutSine);
                t->animation->setStartValue(t->pos());
                t->animation->setKeyValueAt(0.5, item->pos());
                t->animation->setEndValue(arrowsFromList->first()->endItem()->pos());
                t->destroyOnStop();
                t->animation->start();
                startPos = t->pos();
            }
            foreach (Arrow *arrow, *arrowsFromList) {
                QRadialGradient radialGrad(QPointF(0, 0), 4);
                radialGrad.setColorAt(0, Qt::red);
                radialGrad.setColorAt(1, Qt::darkRed);
                Token *t = new Token(arrow->endItem());
                t->setBrush(radialGrad);
                this->addItem(t);
                insertedTokens.append(t);
                t->animation->setDuration(400);
                t->animation->setEasingCurve(QEasingCurve::InOutSine);
                t->animation->setStartValue(startPos);
                t->animation->setKeyValueAt(0.5, item->pos());
                t->animation->setEndValue(arrow->endItem()->pos());
                t->animation->start();
            }
            delete arrowsToList;
            delete arrowsFromList;
        }
        readyTransitions.removeAt(n);
    }
    foreach (Token *token, insertedTokens) {
        token->place()->addToken(token);
    }
}

void DiagramScene::read(QIODevice *device)
{
    QDomDocument domDocument;
    QString errorStr;
    int errorLine;
    int errorColumn;
    if (!domDocument.setContent(device, true, &errorStr, &errorLine,
                                &errorColumn)) {
        return;
    }
    QDomElement root = domDocument.documentElement();
    if (root.tagName() != "PetriNet") {
        return;
    }
    clear();
    QDomNode child = root.firstChild();
    while (!child.isNull()) {
        //parseFolderElement(child);
        if(child.nodeName() == "Place"){
            DiagramItem *item;
            item = new DiagramItem(DiagramItem::Place, myItemMenu);
            item->setBrush(myItemColor);
            addItem(item);
            item->setPos(QPointF(child.attributes().namedItem("x").toAttr().value().toDouble(),
                                 child.attributes().namedItem("y").toAttr().value().toDouble()));
            for(int i=child.attributes().namedItem("tokens").toAttr().value().toInt(); i>0; i--)
                item->addToken();

            emit itemInserted(item);
        }
        if(child.nodeName() == "Transition"){
            DiagramItem *item;
            item = new DiagramItem(DiagramItem::Transition, myItemMenu);
            item->setBrush(myItemColor);
            addItem(item);
            item->setPos(QPointF(child.attributes().namedItem("x").toAttr().value().toDouble(),
                                 child.attributes().namedItem("y").toAttr().value().toDouble()));
            emit itemInserted(item);
        }
        if(child.nodeName() == "Arrow"){
            DiagramItem *startItem=0, *endItem=0;
            double sx = child.attributes().namedItem("startx").toAttr().value().toDouble(),
                   sy = child.attributes().namedItem("starty").toAttr().value().toDouble(),
                   ex = child.attributes().namedItem("endx").toAttr().value().toDouble(),
                   ey = child.attributes().namedItem("endy").toAttr().value().toDouble();
            foreach (QGraphicsItem *item, items()) {
                if (item->type() == DiagramItem::Type){
                    DiagramItem * it = qgraphicsitem_cast<DiagramItem *>(item);
                    if(it->x()==sx&&it->y()==sy)startItem = it;
                    if(it->x()==ex&&it->y()==ey)endItem = it;
                }
            }
            if(startItem&&endItem){
                Arrow *arrow = new Arrow(startItem, endItem, myItemMenu);
                arrow->setColor(myLineColor);
                startItem->addArrow(arrow);
                endItem->addArrow(arrow);
                arrow->setZValue(-1000);
                addItem(arrow);
                arrow->updatePosition();
                if(child.attributes().namedItem("inverted").toAttr().value()=="1")arrow->invert();
                arrow->setBendPoint(child.attributes().namedItem("x").toAttr().value().toDouble(),
                                    child.attributes().namedItem("y").toAttr().value().toDouble());
            }
        }
        child = child.nextSibling();
    }
}

void DiagramScene::write(QIODevice *device)
{
    QDomDocument domDocument;
    QDomElement root = domDocument.createElement("PetriNet");
    foreach (QGraphicsItem *item, items()) {
        if (item->type() == DiagramItem::Type){
            DiagramItem * it = qgraphicsitem_cast<DiagramItem *>(item);
            QString name = it->diagramType()==DiagramItem::Place ? "Place" : "Transition";
            QDomElement node = domDocument.createElement(name);
            QDomAttr at = domDocument.createAttribute("x");
            at.setValue(QString::number(it->x()));
            node.setAttributeNode(at);
            at = domDocument.createAttribute("y");
            at.setValue(QString::number(it->y()));
            node.setAttributeNode(at);
            at = domDocument.createAttribute("tokens");
            at.setValue(QString::number(it->numberOfTokens()));
            node.setAttributeNode(at);
            root.appendChild(node);
        }
    }
    foreach (QGraphicsItem *item, items()) {
        if (item->type() == Arrow::Type){
            Arrow *a = qgraphicsitem_cast<Arrow *>(item);
            QDomElement node = domDocument.createElement("Arrow");
            QDomAttr at = domDocument.createAttribute("x");
            at.setValue(QString::number(a->bendPoint.x()));
            node.setAttributeNode(at);
            at = domDocument.createAttribute("y");
            at.setValue(QString::number(a->bendPoint.y()));
            node.setAttributeNode(at);
            at = domDocument.createAttribute("startx");
            at.setValue(QString::number(a->startItem()->x()));
            node.setAttributeNode(at);
            at = domDocument.createAttribute("starty");
            at.setValue(QString::number(a->startItem()->y()));
            node.setAttributeNode(at);
            at = domDocument.createAttribute("endx");
            at.setValue(QString::number(a->endItem()->x()));
            node.setAttributeNode(at);
            at = domDocument.createAttribute("endy");
            at.setValue(QString::number(a->endItem()->y()));
            node.setAttributeNode(at);
            at = domDocument.createAttribute("inverted");
            at.setValue(a->isInverted()?"1":"0");
            node.setAttributeNode(at);
            root.appendChild(node);
        }
    }
    domDocument.appendChild(root);
    QTextStream out(device);
    domDocument.save(out, 4);
}
