#include "FloorAndProfileViewer.h"

FloorAndProfileViewer::FloorAndProfileViewer()
    : QMainWindow()
{
    mesh = new Mesh();

    objViewer = new ObjectViewer(mesh);
    objViewer->setWindowTitle("Object Viewer");
    objViewer->show();

    centralWidget = new CentralWidget(mesh);
    this->setCentralWidget(centralWidget);

    createMenuBar();

    QObject::connect(this, SIGNAL(closeSignal()), objViewer, SLOT(close()));
    QObject::connect(objViewer, SIGNAL(closeSignal()), this, SLOT(close()));
}

FloorAndProfileViewer::~FloorAndProfileViewer()
{
    delete centralWidget;
    delete openAction;
    delete exitAction;
    delete aboutQtAction;
    delete wireframeAction;
    delete flatAction;
    delete smoothAction;
    delete pointAction;
    delete pointSampledAction;
    delete mesh;
    delete objViewer;
}

void FloorAndProfileViewer::closeEvent(QCloseEvent *event)
{
    emit closeSignal();
    event->accept();
}

void FloorAndProfileViewer::closeViewer()
{
    this->close();
}


void FloorAndProfileViewer::createMenuBar()
{
    createFileMenu();
    createViewMenu();
    createAboutMenu();
}

void FloorAndProfileViewer::createFileMenu()
{
    fileMenu = this->menuBar()->addMenu(tr("&File"));

    openAction = new QAction(tr("&Open"), this);
    openAction->setShortcut(tr("Ctrl+O"));
    fileMenu->addAction(openAction);
    QObject::connect(openAction, SIGNAL(triggered()), this, SLOT(openFile()));

    exitAction = new QAction(tr("&Exit"), this);
    exitAction->setShortcut(tr("Esc"));
    fileMenu->addAction(exitAction);
    QObject::connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
}

void FloorAndProfileViewer::createAboutMenu()
{
    aboutMenu = this->menuBar()->addMenu(tr("&About"));

    aboutQtAction = new QAction(tr("&About Qt"), this);
    aboutMenu->addAction(aboutQtAction);
    QObject::connect(aboutQtAction, SIGNAL(triggered()), this, SLOT(aboutQtMessageBox()));
}

void FloorAndProfileViewer::createViewMenu()
{
    viewMenu = this->menuBar()->addMenu(tr("&View"));

    wireframeAction = new QAction(tr("&Wireframe"), this);
    viewMenu->addAction(wireframeAction);
    QObject::connect(wireframeAction, SIGNAL(triggered()), objViewer, SLOT(wireframeMode()));

    flatAction = new QAction(tr("&Flat"), this);
    viewMenu->addAction(flatAction);
    QObject::connect(flatAction, SIGNAL(triggered()), objViewer, SLOT(flatMode()));

    smoothAction = new QAction(tr("&Smooth"), this);
    viewMenu->addAction(smoothAction);
    QObject::connect(smoothAction, SIGNAL(triggered()), objViewer, SLOT(smoothMode()));

    pointAction = new QAction(tr("&Point"), this);
    viewMenu->addAction(pointAction);
    QObject::connect(pointAction, SIGNAL(triggered()), objViewer, SLOT(pointMode()));

    pointSampledAction = new QAction(tr("&Point Sampled"), this);
    viewMenu->addAction(pointSampledAction);
    QObject::connect(pointSampledAction, SIGNAL(triggered()), objViewer, SLOT(pointSampledMode()));
}

void FloorAndProfileViewer::aboutQtMessageBox()
{
    QMessageBox::aboutQt(this);
}


void FloorAndProfileViewer::openFile()
{
    QString file = QFileDialog::getOpenFileName(this, "Open File", QString(), "Mesh Files (*.obj *.off *.stl)");

    if (file.isEmpty()) {
        return;
    }
    mesh->loadMesh(file);
}
