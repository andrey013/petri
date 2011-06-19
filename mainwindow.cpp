#include <QtGui>
#include <QLabel>

#include "mainwindow.h"
#include "diagramitem.h"
#include "diagramscene.h"
#include "diagramtextitem.h"
#include "diagramtoken.h"
#include "arrow.h"

MainWindow::MainWindow()
{
    QTranslator *appTranslator = new QTranslator(this);
    qApp->installTranslator(appTranslator);
    appTranslator->load("diagramscene_ru.qm", qApp->applicationDirPath());
    createActions();
    createMenus();
    scene = new DiagramScene(itemMenu);
    scene->setSceneRect(QRectF(-2000, -1500, 2000, 1500));

    doPetriStepAction = new QAction(QIcon(":/images/step.png"),
                                   tr("Start"), this);
        doPetriStepAction->setStatusTip(tr("Start"));
        doPetriStepAction->setShortcut(tr("Space"));
        connect(doPetriStepAction, SIGNAL(triggered()),
            scene, SLOT(doPetriStep()));

    connect(scene, SIGNAL(itemSelected(QGraphicsItem*)),
        this, SLOT(itemSelected(QGraphicsItem*)));
    createToolbars();

    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing, true);

    setCentralWidget(view);
    setWindowTitle(tr("Petri Net Creator"));
    setUnifiedTitleAndToolBarOnMac(true);



}

void MainWindow::deleteItem()
{
    foreach (QGraphicsItem *item, scene->selectedItems()) {
        if (item->type() == DiagramItem::Type) {
            qgraphicsitem_cast<DiagramItem *>(item)->removeArrows();
            qgraphicsitem_cast<DiagramItem *>(item)->removeTokens();
        } else if (item->type() == Arrow::Type){
            Arrow *arrow = qgraphicsitem_cast<Arrow *>(item);
            arrow->startItem()->removeArrow(arrow);
            arrow->endItem()->removeArrow(arrow);
        }
        scene->removeItem(item);
    }
}

void MainWindow::addToken()
{
    foreach (QGraphicsItem *item, scene->selectedItems())
        if (item->type() == DiagramItem::Type)
            if (qgraphicsitem_cast<DiagramItem *>(item)->diagramType() == DiagramItem::Place)
                qgraphicsitem_cast<DiagramItem *>(item)->addToken();
}

void MainWindow::removeToken()
{
    foreach (QGraphicsItem *item, scene->selectedItems())
        if (item->type() == DiagramItem::Type)
            if (qgraphicsitem_cast<DiagramItem *>(item)->diagramType() == DiagramItem::Place){
                Token *t = qgraphicsitem_cast<DiagramItem *>(item)->removeToken(0);
                if(t)scene->removeItem(qgraphicsitem_cast<QGraphicsItem *>(t));
            }
}

void MainWindow::invertArrow()
{
    foreach (QGraphicsItem *item, scene->selectedItems())
        if (item->type() == Arrow::Type){
            Arrow *a = qgraphicsitem_cast<Arrow *>(item);
            a->invert();
        }
}

void MainWindow::pointerGroupClicked(int id)
{
    if (id == int(DiagramItem::Place) || id == int(DiagramItem::Transition)) {
        scene->setItemType(DiagramItem::DiagramType(id));
        scene->setMode(DiagramScene::InsertItem);
    } else if (id == int(DiagramItem::Text)) {
        scene->setMode(DiagramScene::InsertText);
    } else {
        scene->setMode(DiagramScene::Mode(DiagramScene::Mode(id)));
    }
}

void MainWindow::currentFontChanged(const QFont &)
{
    handleFontChange();
}

void MainWindow::fontSizeChanged(const QString &)
{
    handleFontChange();
}

void MainWindow::sceneScaleChanged(const QString &scale)
{
    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;
    QMatrix oldMatrix = view->matrix();
    view->resetMatrix();
    view->translate(oldMatrix.dx(), oldMatrix.dy());
    view->scale(newScale, newScale);
}

void MainWindow::textColorChanged()
{
    textAction = qobject_cast<QAction *>(sender());
    fontColorToolButton->setIcon(createColorToolButtonIcon(
                ":/images/textpointer.png",
                qVariantValue<QColor>(textAction->data())));
    textButtonTriggered();
}

void MainWindow::itemColorChanged()
{
    fillAction = qobject_cast<QAction *>(sender());
    fillColorToolButton->setIcon(createColorToolButtonIcon(
                 ":/images/floodfill.png",
                 qVariantValue<QColor>(fillAction->data())));
    fillButtonTriggered();
}

void MainWindow::lineColorChanged()
{
    lineAction = qobject_cast<QAction *>(sender());
    lineColorToolButton->setIcon(createColorToolButtonIcon(
                 ":/images/linecolor.png",
                 qVariantValue<QColor>(lineAction->data())));
    lineButtonTriggered();
}

void MainWindow::textButtonTriggered()
{
    scene->setTextColor(qVariantValue<QColor>(textAction->data()));
}

void MainWindow::fillButtonTriggered()
{
    scene->setItemColor(qVariantValue<QColor>(fillAction->data()));
}

void MainWindow::lineButtonTriggered()
{
    scene->setLineColor(qVariantValue<QColor>(lineAction->data()));
}

void MainWindow::handleFontChange()
{
    QFont font = fontCombo->currentFont();
    font.setPointSize(fontSizeCombo->currentText().toInt());
    font.setWeight(boldAction->isChecked() ? QFont::Bold : QFont::Normal);
    font.setItalic(italicAction->isChecked());
    font.setUnderline(underlineAction->isChecked());

    scene->setFont(font);
}

void MainWindow::itemSelected(QGraphicsItem *item)
{
    DiagramTextItem *textItem =
    qgraphicsitem_cast<DiagramTextItem *>(item);

    QFont font = textItem->font();
    QColor color = textItem->defaultTextColor();
    fontCombo->setCurrentFont(font);
    fontSizeCombo->setEditText(QString().setNum(font.pointSize()));
    boldAction->setChecked(font.weight() == QFont::Bold);
    italicAction->setChecked(font.italic());
    underlineAction->setChecked(font.underline());
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Petri Net Creator"),
                       tr("The <b>Petri Net Creator</b> "
                          "allows to build Petri nets"));
}

void MainWindow::createActions()
{
    deleteAction = new QAction(QIcon(":/images/delete.png"),
                               tr("&Delete"), this);
    deleteAction->setShortcut(tr("Delete"));
    deleteAction->setStatusTip(tr("Delete item from diagram"));
    connect(deleteAction, SIGNAL(triggered()),
        this, SLOT(deleteItem()));

    addTokenAction = new QAction(QIcon(":/images/add.png"),
                               tr("&Add token"), this);
    addTokenAction->setStatusTip(tr("Place token on diagram item"));
    connect(addTokenAction, SIGNAL(triggered()),
        this, SLOT(addToken()));

    removeTokenAction = new QAction(QIcon(":/images/remove.png"),
                               tr("&Remove token"), this);
    removeTokenAction->setStatusTip(tr("Remove token from diagram item"));
    connect(removeTokenAction, SIGNAL(triggered()),
        this, SLOT(removeToken()));

    invertArrowAction = new QAction(QIcon(":/images/italic.png"),
                               tr("&Invert Arrow"), this);
    invertArrowAction->setStatusTip(tr("Invert Arrow"));
    connect(invertArrowAction, SIGNAL(triggered()),
        this, SLOT(invertArrow()));

    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Quit program"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

    openAction = new QAction(tr("Open file..."), this);
    openAction->setShortcuts(QKeySequence::Open);
    openAction->setStatusTip(tr("Open file..."));
    connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));

    saveAction = new QAction(tr("Save file as..."), this);
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(tr("Save file as..."));
    connect(saveAction, SIGNAL(triggered()), this, SLOT(saveFile()));

    boldAction = new QAction(tr("Bold"), this);
    boldAction->setCheckable(true);
    QPixmap pixmap(":/images/bold.png");
    boldAction->setIcon(QIcon(pixmap));
    boldAction->setShortcut(tr("Ctrl+B"));
    connect(boldAction, SIGNAL(triggered()),
            this, SLOT(handleFontChange()));

    italicAction = new QAction(QIcon(":/images/italic.png"),
                               tr("Italic"), this);
    italicAction->setCheckable(true);
    italicAction->setShortcut(tr("Ctrl+I"));
    connect(italicAction, SIGNAL(triggered()),
            this, SLOT(handleFontChange()));

    underlineAction = new QAction(QIcon(":/images/underline.png"),
                                  tr("Underline"), this);
    underlineAction->setCheckable(true);
    underlineAction->setShortcut(tr("Ctrl+U"));
    connect(underlineAction, SIGNAL(triggered()),
            this, SLOT(handleFontChange()));

    aboutAction = new QAction(tr("A&bout"), this);
    aboutAction->setShortcut(tr("Ctrl+B"));
    connect(aboutAction, SIGNAL(triggered()),
            this, SLOT(about()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);
    fileMenu->addAction(saveAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    itemMenu = menuBar()->addMenu(tr("&Item"));
    itemMenu->addAction(addTokenAction);
    itemMenu->addAction(removeTokenAction);
    itemMenu->addAction(invertArrowAction);
    itemMenu->addAction(deleteAction);

    aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutAction);
}

void MainWindow::createToolbars()
{
    fontCombo = new QFontComboBox();
    fontSizeCombo = new QComboBox();
    connect(fontCombo, SIGNAL(currentFontChanged(QFont)),
            this, SLOT(currentFontChanged(QFont)));

    fontSizeCombo = new QComboBox;
    fontSizeCombo->setEditable(true);
    for (int i = 8; i < 30; i = i + 2)
        fontSizeCombo->addItem(QString().setNum(i));
    QIntValidator *validator = new QIntValidator(2, 64, this);
    fontSizeCombo->setValidator(validator);
    connect(fontSizeCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(fontSizeChanged(QString)));

    fontColorToolButton = new QToolButton;
    fontColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    fontColorToolButton->setMenu(createColorMenu(SLOT(textColorChanged()),
                                                 Qt::black));
    textAction = fontColorToolButton->menu()->defaultAction();
    fontColorToolButton->setIcon(createColorToolButtonIcon(
    ":/images/textpointer.png", Qt::black));
    fontColorToolButton->setAutoFillBackground(true);
    connect(fontColorToolButton, SIGNAL(clicked()),
            this, SLOT(textButtonTriggered()));

    fillColorToolButton = new QToolButton;
    fillColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    fillColorToolButton->setMenu(createColorMenu(SLOT(itemColorChanged()),
                         Qt::white));
    fillAction = fillColorToolButton->menu()->defaultAction();
    fillColorToolButton->setIcon(createColorToolButtonIcon(
    ":/images/floodfill.png", Qt::white));
    connect(fillColorToolButton, SIGNAL(clicked()),
            this, SLOT(fillButtonTriggered()));

    lineColorToolButton = new QToolButton;
    lineColorToolButton->setPopupMode(QToolButton::MenuButtonPopup);
    lineColorToolButton->setMenu(createColorMenu(SLOT(lineColorChanged()),
                                 Qt::black));
    lineAction = lineColorToolButton->menu()->defaultAction();
    lineColorToolButton->setIcon(createColorToolButtonIcon(
        ":/images/linecolor.png", Qt::black));
    connect(lineColorToolButton, SIGNAL(clicked()),
            this, SLOT(lineButtonTriggered()));

    QToolButton *pointerButton = new QToolButton;
    pointerButton->setCheckable(true);
    pointerButton->setChecked(true);
    pointerButton->setIcon(QIcon(":/images/pointer.png"));
    QToolButton *linePointerButton = new QToolButton;
    linePointerButton->setCheckable(true);
    linePointerButton->setIcon(QIcon(":/images/linepointer.png"));
    QToolButton *placeButton = new QToolButton;
    placeButton->setCheckable(true);
    placeButton->setIcon(QIcon(":/images/placepointer.png"));
    QToolButton *transitButton = new QToolButton;
    transitButton->setCheckable(true);
    transitButton->setIcon(QIcon(":/images/transitpointer.png"));
    QToolButton *textButton = new QToolButton;
    textButton->setCheckable(true);
    textButton->setIcon(QIcon(":/images/textpointer.png"));

    pointerTypeGroup = new QButtonGroup;
    pointerTypeGroup->addButton(pointerButton, int(DiagramItem::Move));
    pointerTypeGroup->addButton(linePointerButton,
                                int(DiagramItem::Line));
    pointerTypeGroup->addButton(placeButton,
                                int(DiagramItem::Place));
    pointerTypeGroup->addButton(transitButton,
                                int(DiagramItem::Transition));
    pointerTypeGroup->addButton(textButton,
                                int(DiagramItem::Text));

    connect(pointerTypeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(pointerGroupClicked(int)));



    sceneScaleCombo = new QComboBox;
    QStringList scales;
    scales << tr("25%") << tr("50%") << tr("100%") << tr("200%") << tr("400%");
    sceneScaleCombo->addItems(scales);
    sceneScaleCombo->setCurrentIndex(2);
    connect(sceneScaleCombo, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(sceneScaleChanged(QString)));

    pointerToolbar = addToolBar(tr("Pointer type"));
    pointerToolbar->addWidget(pointerButton);
    pointerToolbar->addWidget(placeButton);
    pointerToolbar->addWidget(transitButton);
    pointerToolbar->addWidget(linePointerButton);
    pointerToolbar->addWidget(textButton);
    pointerToolbar->addWidget(sceneScaleCombo);
    pointerToolbar->addAction(doPetriStepAction);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(addTokenAction);
    editToolBar->addAction(removeTokenAction);
    editToolBar->addAction(invertArrowAction);
    editToolBar->addAction(deleteAction);

    colorToolBar = addToolBar(tr("Color"));
    colorToolBar->addWidget(fontColorToolButton);
    colorToolBar->addWidget(fillColorToolButton);
    colorToolBar->addWidget(lineColorToolButton);

    textToolBar = addToolBar(tr("Font"));
    textToolBar->addWidget(fontCombo);
    textToolBar->addWidget(fontSizeCombo);
    textToolBar->addAction(boldAction);
    textToolBar->addAction(italicAction);
    textToolBar->addAction(underlineAction);


}

QMenu *MainWindow::createColorMenu(const char *slot, QColor defaultColor)
{
    QList<QColor> colors;
    colors << Qt::black << Qt::white << Qt::red << Qt::blue << Qt::yellow;
    QStringList names;
    names << tr("black") << tr("white") << tr("red") << tr("blue") << tr("yellow");

    QMenu *colorMenu = new QMenu;
    for (int i = 0; i < colors.count(); ++i) {
        QAction *action = new QAction(names.at(i), this);
        action->setData(colors.at(i));
        action->setIcon(createColorIcon(colors.at(i)));
        connect(action, SIGNAL(triggered()),
                this, slot);
        colorMenu->addAction(action);
        if (colors.at(i) == defaultColor) {
            colorMenu->setDefaultAction(action);
        }
    }
    return colorMenu;
}

QIcon MainWindow::createColorToolButtonIcon(const QString &imageFile,
                        QColor color)
{
    QPixmap pixmap(50, 80);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    QPixmap image(imageFile);
    QRect target(0, 0, 50, 60);
    QRect source(0, 0, 42, 42);
    painter.fillRect(QRect(0, 60, 50, 80), color);
    painter.drawPixmap(target, image, source);

    return QIcon(pixmap);
}

QIcon MainWindow::createColorIcon(QColor color)
{
    QPixmap pixmap(20, 20);
    QPainter painter(&pixmap);
    painter.setPen(Qt::NoPen);
    painter.fillRect(QRect(0, 0, 20, 20), color);

    return QIcon(pixmap);
}

void MainWindow::openFile()
{
    QString fileName =
        QFileDialog::getOpenFileName(this, tr("Open"),
            QDir::currentPath(),
            tr("Petri net files(*.xml)"));
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Petri net creator"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    scene->read(&file);
    file.close();
}

void MainWindow::saveFile()
{
    QString fileName =
        QFileDialog::getSaveFileName(this, tr("Save"),
            QDir::currentPath(),
            tr("Petri net files(*.xml)"));
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Petri net creator"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }
    scene->write(&file);
    file.close();
}
