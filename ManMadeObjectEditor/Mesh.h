#ifndef MESH_H
#define MESH_H

#include <QGLViewer/qglviewer.h>
#include <vector>
#include <iostream>
#include <QObject>
#include <cmath>

#include "FloorVertex.h"
#include "FloorPlanAndProfileExtractor.h"
#include "Utils.h"

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>

// étant donnée un floor plan, cette classe va pouvoir construire le mesh à afficher avec openGL

class Mesh : public QObject
{
    Q_OBJECT

    typedef OpenMesh::PolyMesh_ArrayKernelT<>  OMMesh;
public:
    Mesh();
    ~Mesh();
    const std::vector<qglviewer::Vec *>& getTriangles();
    // moreSample: if we want more sample to be rendered than the number of point we really have
    // spaceBetweenSample: space between sample
    std::vector<qglviewer::Vec*>* getPoints(bool moreSample = false, float spaceBetweenSample = 0.1f);

    void update();
    unsigned int getFloorPlanSize();
    void incrementFloorPlanSize();
    void decrementFloorPlanSize();

    Profile* getCurrentProfile();
    void setCurrentProfile(Profile* p);
    void loadMesh(QString fileName);
    FloorVertex* getFloorPlan();
    void setFloorPlan(FloorVertex* vertex);
    void setUpdateOnMesh();
    void setLongUpdateOnMesh(bool b);

signals:
    void newFloorPlan();

private:
    std::vector<qglviewer::Vec *> triangles;
    FloorVertex* floorPlan; // linked list, can iterate on it using the neighbor
    Profile* currentProfile;
    OMMesh* inputMesh;
    bool updateOnMesh;
    bool longUpdateOnMesh;
    unsigned int floorPlanSize;

    void getPointBasic(std::vector<qglviewer::Vec*>* points);
    void getPointWithAdditionnalSampledPoint(std::vector<qglviewer::Vec*>* points, float spaceBetweenSample);
    void computeNormals();

    std::vector<qglviewer::Vec*>* points;
};

#endif // MESH_H
