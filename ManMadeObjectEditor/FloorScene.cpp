#include "FloorScene.h"
#include <iostream>

const int FloorScene::vertexRadius(3);

FloorScene::FloorScene(Mesh* mesh)
    : QGraphicsScene(), ctrl_pressed(false), shift_pressed(false), mesh(mesh)
    , currentlyMovingVertex(0), isVertexMoving(false)
{
    this->setSceneRect(QRectF(0, 0, 400, 600));
}

FloorScene::~FloorScene()
{

}

void FloorScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
    if (mouseEvent->button() ==  Qt::RightButton){
        if(ctrl_pressed){
            addVertex(mouseEvent->lastScenePos().toPoint());
        } else if (shift_pressed) {
            removeVertex();
        } else {
            unsigned int size = mesh->getFloorPlanSize();

            // we have have at least one vertex on the floor plan, if we click on it, we can move it
            // and we draw his profile
            if (size != 0) {
                moveVertex();
            // else if we do not have any vertices on the floor plan, we create a floor plan
            } else {
                QPoint mousePos = mouseEvent->lastScenePos().toPoint();
                // no point defined thus we add an initial geometric structure
                basicCircle(&mousePos, 10);
            }
        }
    }
}

void FloorScene::moveVertex()
{
    unsigned int size = mesh->getFloorPlanSize();
    // find which vertex we have selected
    Vertex* currentVertex = mesh->getFloorPlan();
    for (unsigned int i(0); i < size; ++i) {
        QGraphicsEllipseItem* ellipse = currentVertex->getEllipse();
        if (ellipse->isUnderMouse()) {
            isVertexMoving = true;
            currentlyMovingVertex = currentVertex;
            newProfileSelected(((FloorVertex*)currentVertex)->getProfile());

            // tell the mesh to generate new point/triangle
            mesh->setLongUpdateOnMesh(true);
            break;
        }
        currentVertex = currentVertex->getNeighbor2();
    }
}

void FloorScene::removeVertex()
{
    //remove point if clicked on a point
    FloorVertex* floorplan = mesh->getFloorPlan();
    unsigned int size = mesh->getFloorPlanSize();

    if (size <= 3) {
        return;
    }

    Vertex* currentVertex = floorplan;

    //find if a vertex is under the mouse
    for (unsigned int i(0); i < size; ++i) {
        QGraphicsEllipseItem* ellipse = currentVertex->getEllipse();

        if (ellipse->isUnderMouse()) {
            // first set the floorplan pointer in mesh to be the neighbor of the
            // currently deleted vertex. Because it can be the one pointed in
            // the mesh class, used to iterate over the floorplan
            mesh->setFloorPlan((FloorVertex*)currentVertex->getNeighbor2());

            // we find one vertex to remove under the mouse, we remove it
            QGraphicsLineItem* newEdge = currentVertex->removeVertex();
            QGraphicsLineItem* oldEdge1 = currentVertex->getEdge1();
            QGraphicsLineItem* oldEdge2 = currentVertex->getEdge2();
            this->removeItem(ellipse);
            this->removeItem(oldEdge1);
            this->removeItem(oldEdge2);

            this->addItem(newEdge);

            delete oldEdge1;
            delete oldEdge2;
            delete ellipse;
            delete currentVertex;

            mesh->decrementFloorPlanSize();

            // tell the mesh to generate new point/triangle
            mesh->setUpdateOnMesh();

            break;
        }
        currentVertex = currentVertex->getNeighbor2();
    }
}

void FloorScene::addVertex(QPoint mousePos)
{
    //add point if clicked on an edge
    FloorVertex* floorplan = mesh->getFloorPlan();
    unsigned int size = mesh->getFloorPlanSize();
    FloorVertex* previousVertex = floorplan;
    FloorVertex* nextVertex;
    float x(0.0f);
    float y(0.0f);
    QGraphicsLineItem * currentEdge(0);
    for (unsigned int i(0); i < size; ++i) {
        if (previousVertex->getEdge2()->isUnderMouse()){
           nextVertex = (FloorVertex*)previousVertex->getNeighbor2();
           currentEdge = previousVertex->getEdge2();
           x = mousePos.x();
           y = mousePos.y();
           break;
        }
        previousVertex = (FloorVertex*)previousVertex->getNeighbor2();
    }

    if(currentEdge != 0){
        QGraphicsEllipseItem* ellipse = new QGraphicsEllipseItem(x - vertexRadius, y - vertexRadius, vertexRadius * 2.0f, vertexRadius * 2.0f);
        adjustCoordinatesSceneTo3D(x, y);

        //build the new vertex and add the edges between the new neighbour
        FloorVertex* newVertex = new FloorVertex(x,y, previousVertex->getProfile());//comme choisir quel profil mettre ca c'est tres interessant...
        newVertex->setEllipse(ellipse);


        QGraphicsLineItem* edge1 = previousVertex->replaceNeighbour(nextVertex, newVertex);
        QGraphicsLineItem* edge2 = nextVertex->replaceNeighbour(previousVertex, newVertex);


        //set all neighbour/edges of the new vertex
        newVertex->setNeighbor1(previousVertex); //addNeighbor
        newVertex->setEdge1(edge1);
        newVertex->setNeighbor2(nextVertex); //addNeighbor
        newVertex->setEdge2(edge2);


        //finally add the new vertex to the mesh, show the ellipse and delete the old edge
        mesh->incrementFloorPlanSize();
        this->addItem(ellipse);
        this->addItem(edge1);
        this->addItem(edge2);
        this->removeItem(currentEdge);
        delete currentEdge;
        // tell the mesh to generate new point/triangle
        mesh->setUpdateOnMesh();
    }
}

void FloorScene::keyPressEvent(QKeyEvent* keyEvent)
{
    switch(keyEvent->key())
    {
        case Qt::Key_Control:
            ctrl_pressed = true;
            break;
        case Qt::Key_Shift:
            shift_pressed = true;
            break;
    }
}

void FloorScene::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Control:
            ctrl_pressed = false;
            break;
        case Qt::Key_Shift:
            shift_pressed = false;
            break;
    }
}

void FloorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{

    if (event->button() ==  Qt::RightButton) {
        if (isVertexMoving) {
            isVertexMoving = false;
            currentlyMovingVertex = 0;
            // tell the mesh to generate new point/triangle
            mesh->setLongUpdateOnMesh(false);
        }
    }
}

void FloorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (isVertexMoving) {
        QPoint mousePos = event->lastScenePos().toPoint();
        float x = mousePos.x();
        float y = mousePos.y();


        QGraphicsEllipseItem* ellipse = currentlyMovingVertex->getEllipse();
        ellipse->setRect(x - vertexRadius, y - vertexRadius, vertexRadius * 2.0f, vertexRadius * 2.0f);

        Vertex* neighbor1 = currentlyMovingVertex->getNeighbor1();
        QGraphicsLineItem* edge1 = currentlyMovingVertex->getEdge1();
        edge1->setLine(ellipse->rect().center().rx(), ellipse->rect().center().ry(),
                       neighbor1->getEllipse()->rect().center().rx(), neighbor1->getEllipse()->rect().center().ry());

        Vertex* neighbor2 = currentlyMovingVertex->getNeighbor2();
        QGraphicsLineItem* edge2 = currentlyMovingVertex->getEdge2();
        edge2->setLine(ellipse->rect().center().rx(), ellipse->rect().center().ry(),
                       neighbor2->getEllipse()->rect().center().rx(), neighbor2->getEllipse()->rect().center().ry());

        adjustCoordinatesSceneTo3D(x, y);
        currentlyMovingVertex->setX(x);
        currentlyMovingVertex->setY(y);
    }
}

void FloorScene::basicCircle(QPoint *mousePos, int numSample)
{
    Profile* commonProfile = new Profile(false);
    ProfileDestructorManager::putProfile(commonProfile);

    int radius(100);
    float x(0.0f);
    float y(0.0f);
    float alpha(0.0f);

    FloorVertex* first;
    FloorVertex* prev;
    FloorVertex* current;

    for(int i(0); i < numSample; i++)
    {
        alpha = 2.0f * M_PI * (float)(i) / (float)(numSample);
        x = mousePos->x() + radius * cos(alpha);
        y = mousePos->y() + radius * sin(alpha);

        adjustCoordinatesSceneTo3D(x, y);

        if (i == 0) {
            first = new FloorVertex(x, y, commonProfile);
            mesh->setFloorPlan(first);
            mesh->incrementFloorPlanSize();
            prev = first;
        } else {
            current = new FloorVertex(x, y, commonProfile);
            mesh->incrementFloorPlanSize();
            prev->setNeighbor2(current);//addNeighbor
            current->setNeighbor1(prev);//addNeighbor

            //this->addLine(prev->getX(), prev->getY(), current->getX(), current->getY());

            prev = current;
        }
    }
    current->setNeighbor2(first);//addNeighbor
    first->setNeighbor1(current);//addNeighbor
    newProfileSelected(commonProfile);

    loadFloorPlan();
    //this->addLine(first->getX(), first->getY(), current->getX(), current->getY());

    // tell the mesh to generate new point/triangle
    mesh->setUpdateOnMesh();
}

void FloorScene::newProfileSelected(Profile* p)
{
    mesh->setCurrentProfile(p);
    emit newProfileSelected();
}

void FloorScene::adjustCoordinatesSceneTo3D(float& x, float& y)
{
    // from scene coord to 3D
    QRectF thisSize = this->sceneRect();
    float width = thisSize.width();
    float height = thisSize.height();
    float width_2 = width/ 2.0f;
    float height_2 = height / 2.0f;

    y = height_2  - y;
    x = -width_2 + x;

    x = x/width_2;
    y = y/width_2;
}

void FloorScene::loadFloorPlan() {
    this->clear();

    FloorVertex* floorplan = mesh->getFloorPlan();
    unsigned int size = mesh->getFloorPlanSize();
    float x(0.0f);
    float y(0.0f);
    Vertex* currentVertex = floorplan;

    for (unsigned int i(0); i < size; ++i) {
        x = currentVertex->getX();
        y = currentVertex->getY();
        adjustCoordinates3DToScene(x, y);
        if(currentVertex->getEllipse() == 0){
            currentVertex->setEllipse(new QGraphicsEllipseItem(x - vertexRadius, y - vertexRadius, vertexRadius * 2.0f, vertexRadius * 2.0f));
            this->addItem(currentVertex->getEllipse());

        } else {
            std::cerr << "something strange happen, try to reload an already loaded floorplan mabye is ok and i shouldn't do anything";
        }
        currentVertex = currentVertex->getNeighbor2();
    }

    currentVertex = floorplan;
    for (unsigned int i(0); i < size; ++i) {
        if(currentVertex->addEdge1()){
            this->addItem(currentVertex->getEdge1());
        }
        if(currentVertex->addEdge2()){
           this->addItem(currentVertex->getEdge2());
        }
        currentVertex = currentVertex->getNeighbor2();
    }
    // tell the mesh to generate new point/triangle
    mesh->setUpdateOnMesh();
}

void FloorScene::adjustCoordinates3DToScene(float& x, float& y)
{
    // from 3D to Scene coord
    QRectF thisSize = this->sceneRect();
    float width = thisSize.width();
    float height = thisSize.height();

    float width_2 = width/ 2.0f;
    float height_2 = height / 2.0f;

    x = x * width_2;
    y = y * width_2;

    y = height_2 - y;
    x = x + width_2;
}
