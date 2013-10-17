#include "Mesh.h"

Mesh::Mesh(): inputMesh(0), floorPlan(0), updateOnMesh(false), longUpdateOnMesh(false), floorPlanSize(0)
{
    points = new std::vector<qglviewer::Vec*>;


    // simple test
    triangles.push_back(new qglviewer::Vec(-1.0f, 0.0f, 0.5f));
    triangles.push_back(new qglviewer::Vec(-1.0f, 1.0f, 0.5f));
    triangles.push_back(new qglviewer::Vec(1.0f, 0.0f, 0.5f));

    triangles.push_back(new qglviewer::Vec(1.0f, 0.0f, 0.5f));
    triangles.push_back(new qglviewer::Vec(-1.0f, 1.0f, 0.5f));
    triangles.push_back(new qglviewer::Vec(1.0f, 1.0f, 0.5f));

}

Mesh::~Mesh()
{
    unsigned int size = triangles.size();
    for( unsigned int i(0); i < size; ++i)
    {
        qglviewer::Vec* tmp = triangles[i];
        delete tmp;
    }

    Vertex* currentVertex = floorPlan;
    for( unsigned int i(0); i < floorPlanSize; ++i)
    {
        Vertex* next = currentVertex->getNeighbor2();
        if (currentVertex != 0) {
            //Not needed because (Qt documentation):
            // "Removes and deletes all items from the scene object before destroying the scene object.
            // The scene object is removed from the application's global scene list, and it is removed
            // from all associated views."

            /*if (currentVertex->getEdge2() != 0) {
                delete currentVertex->getEdge2();
            }
            if (currentVertex->getEllipse() != 0){
                delete currentVertex->getEllipse();
            }*/
            delete currentVertex;
            currentVertex = 0;
        }
        currentVertex = next;
    }

    delete points;
}

void Mesh::setFloorPlan(FloorVertex* vertex) {
   floorPlan = vertex;
}

const std::vector<qglviewer::Vec *>& Mesh::getTriangles()
{
    // va devoir appliquer algorithme que etant donnée les floorVertex/edges et les profile, reconstruire les triangles
    return triangles;
}

std::vector<qglviewer::Vec*>* Mesh::getPoints(bool moreSample, float ds)
{

    if (!updateOnMesh && !longUpdateOnMesh) {
        return points;
    }

    unsigned int size = points->size();
    for( unsigned int i(0); i < size; ++i)
    {
        qglviewer::Vec* tmp = (*points)[i];
        delete tmp;
    }

    points->clear();

    if (floorPlanSize == 0) {
        return points;
    }

    if (moreSample) {
        getPointWithAdditionnalSampledPoint(points, ds);
    } else {
        getPointBasic(points);
    }

    updateOnMesh = false;

    return points;
}

void Mesh::getPointBasic(std::vector<qglviewer::Vec*>* points)
{
    float centerX(0.0f);
    float centerY(0.0f);

    Vertex* tmp = floorPlan;
    for(unsigned int i(0); i < floorPlanSize; ++i) {
        centerX += tmp->getX();
        centerY += tmp->getY();
        tmp = tmp->getNeighbor2();
    }

    centerX = centerX / (float)(floorPlanSize);
    centerY = centerY / (float)(floorPlanSize);

    FloorVertex* floorVertexTmp = floorPlan;

    for(unsigned int i(0); i < floorPlanSize; ++i) {

        Profile* profile = floorVertexTmp->getProfile();
        if (profile != 0) {
           Vertex* pVertex = profile->getProfileVertex();
            while(pVertex != 0)
            {
                float w = pVertex->getX();
                float z = pVertex->getY();

                float x = floorVertexTmp->getX()* (1.0f - w) + centerX * w;
                float y = floorVertexTmp->getY()* (1.0f - w) + centerY * w;

                points->push_back(new qglviewer::Vec(x, y, z));
                pVertex = pVertex->getNeighbor2();
            }
        }
        floorVertexTmp = (FloorVertex*)floorVertexTmp->getNeighbor2();
    }
}

// First try
/*void Mesh::getPointWithAdditionnalSampledPoint(std::vector<qglviewer::Vec*>& points, float ds)
{
    // very basic. Let say we have two vertices on the floor plan v1 and v2,
    // we interpolate new vertices between these two and take the profile of v1
    // and "associate" this profile to the new vertices. This is the idea
    // used to add new sample in the floor plan
    // To add new sample for the profile, we take vertices of the profile
    // two by two and interpolate
    // This method is only used to show more points on the 3D rendering
    float centerX(0.0f);
    float centerY(0.0f);

    Vertex* tmp = floorPlan;
    for(unsigned int i(0); i < floorPlanSize; ++i) {
        centerX += tmp->getX();
        centerY += tmp->getY();
        tmp = tmp->getNeighbor2();
    }

    centerX = centerX / (float)(floorPlanSize);
    centerY = centerY / (float)(floorPlanSize);

    FloorVertex* floorVertexTmp = floorPlan;
    FloorVertex* nextFloorVertexTmp = (FloorVertex*)floorVertexTmp->getNeighbor2();

    if (nextFloorVertexTmp == 0) {
        return;
    }

    for(unsigned int i(0); i < floorPlanSize; ++i) {

        Profile* profile = floorVertexTmp->getProfile();
        if (profile != 0) {
           Vertex* pVertex = profile->getProfileVertex();

           float nextX = nextFloorVertexTmp->getX();
           float nextY = nextFloorVertexTmp->getY();


            while(pVertex->getNeighbor2() != 0)
            {
                float currentW = pVertex->getX();

                Vertex* nextPVertex = pVertex->getNeighbor2();
                float nextW = nextPVertex->getX();
                float currentZ = pVertex->getY();
                float nextZ = nextPVertex->getY();

                float t = 0.0f;
                while(t < 1.0f) {
                    float t_1 = 1.0f - t;
                    float w = currentW * t + nextW * t_1;
                    float z = currentZ * t + nextZ * t_1;

                    // we take a new sample at every ds between two neighbor
                    // and use the profile of the first neighbor
                    float tt = 0.0f;
                    while(tt < 1.0f) {
                        float tt_1 = 1.0f - tt;
                        float sampledX = floorVertexTmp->getX() * tt + tt_1 * nextX;
                        float sampledY = floorVertexTmp->getY() * tt + tt_1 * nextY;

                        float newX = sampledX * (1.0f - w) + centerX * w;
                        float newY = sampledY * (1.0f - w) + centerY * w;

                        points.push_back(new qglviewer::Vec(newX, newY, z));

                        tt += ds;
                    }
                    t += ds;
                }
                pVertex = pVertex->getNeighbor2();
            }
        }
        floorVertexTmp = (FloorVertex*)floorVertexTmp->getNeighbor2();
        nextFloorVertexTmp = (FloorVertex*)nextFloorVertexTmp->getNeighbor2();
    }
}*/

void Mesh::getPointWithAdditionnalSampledPoint(std::vector<qglviewer::Vec*>* points, float ds)
{
    // very basic. Let say we have two vertices on the floor plan v1 and v2,
    // we interpolate new vertices between these two and take the profile of v1
    // and "associate" this profile to the new vertices. This is the idea
    // used to add new sample in the floor plan
    // To add new sample for the profile, we take vertices of the profile
    // two by two and interpolate
    // This method is only used to show more points on the 3D rendering
    float centerX(0.0f);
    float centerY(0.0f);

    Vertex* tmp = floorPlan;
    for(unsigned int i(0); i < floorPlanSize; ++i) {
        centerX += tmp->getX();
        centerY += tmp->getY();
        tmp = tmp->getNeighbor2();
    }

    centerX = centerX / (float)(floorPlanSize);
    centerY = centerY / (float)(floorPlanSize);

    FloorVertex* floorVertexTmp = floorPlan;
    FloorVertex* nextFloorVertexTmp = (FloorVertex*)floorVertexTmp->getNeighbor2();

    if (nextFloorVertexTmp == 0) {
        return;
    }

    for(unsigned int i(0); i < floorPlanSize; ++i) {

        Profile* profile = floorVertexTmp->getProfile();
        if (profile != 0) {
           Vertex* pVertex = profile->getProfileVertex();

           float nextX = nextFloorVertexTmp->getX();
           float nextY = nextFloorVertexTmp->getY();
           float currentX = floorVertexTmp->getX();
           float currentY = floorVertexTmp->getY();

           // we take a new sample at every ds between two neighbor
           // and use the profile of the first neighbor
            while(pVertex->getNeighbor2() != 0)
            {
                float currentW = pVertex->getX();

                Vertex* nextPVertex = pVertex->getNeighbor2();
                float nextW = nextPVertex->getX();
                float currentZ = pVertex->getY();
                float nextZ = nextPVertex->getY();

                float dirW = nextW - currentW;
                float dirZ = nextZ - currentZ;
                float norm = std::sqrt(dirW * dirW + dirZ * dirZ);
                dirW = dirW / norm;
                dirZ = dirZ / norm;

                float sampledW(0.0f);
                float newZ(0.0f);
                float t = 0.0f;

                float deltaW = sampledW - nextW;
                float deltaZ = newZ - nextZ;

                do {
                    sampledW = currentW + dirW * t;
                    newZ = currentZ + dirZ * t;

                    deltaW = sampledW - nextW;
                    deltaZ = newZ - nextZ;

                    float tt = 0.0f;
                    float sampledX(0.0f);
                    float sampledY(0.0f);
                    float dirX = nextX - currentX;
                    float dirY = nextY - currentY;
                    norm = std::sqrt(dirX * dirX + dirY * dirY);
                    dirX = dirX / norm;
                    dirY = dirY / norm;


                    float deltaX = sampledX - nextX;
                    float deltaY = sampledY - nextY;
                    do {
                        sampledX = currentX + dirX * tt;
                        sampledY = currentY + dirY * tt;

                        deltaX = sampledX - nextX;
                        deltaY = sampledY - nextY;

                        float newX = sampledX * (1.0f - sampledW) + centerX * sampledW;
                        float newY = sampledY * (1.0f - sampledW) + centerY * sampledW;

                        points->push_back(new qglviewer::Vec(newX, newY, newZ));

                        tt += ds;
                    // if the distance beween the samplend X,Y and next X,Y is small, stop
                    } while(std::sqrt(deltaX*deltaX + deltaY*deltaY) > ds);

                    t += ds;
                // if the distance beween the samplend W,Z and next W,Z is small, stop
                } while (std::sqrt(deltaW*deltaW + deltaZ*deltaZ) > ds);

                pVertex = pVertex->getNeighbor2();
            }
        }
        floorVertexTmp = (FloorVertex*)floorVertexTmp->getNeighbor2();
        nextFloorVertexTmp = (FloorVertex*)nextFloorVertexTmp->getNeighbor2();
    }
}


void Mesh::update()
{
    // hum surement que va reconstruire la liste de triangle en fonction du nouvelle input de point/profile donnée
    qglviewer::Vec* tmp = triangles[0];
    tmp->x -= 0.0001f;
}

unsigned int Mesh::getFloorPlanSize()
{
    return floorPlanSize;
}

void Mesh::incrementFloorPlanSize()
{
    floorPlanSize++;
}

void Mesh::decrementFloorPlanSize()
{
    floorPlanSize--;
}

Profile* Mesh::getCurrentProfile()
{
    return currentProfile;
}

void Mesh::setCurrentProfile(Profile* p)
{
    currentProfile = p;
}

void Mesh::loadMesh(QString fileName)
{
    if(inputMesh != 0)
    {
        delete inputMesh;
    }
    inputMesh = new OMMesh;

    Vertex* currentVertex = floorPlan;
    for( unsigned int i(0); i < floorPlanSize; ++i)
    {
        Vertex* next = currentVertex->getNeighbor2();
        delete currentVertex->getEdge2();
        delete currentVertex->getEllipse();
        delete currentVertex;
        currentVertex = next;
    }
    floorPlanSize = 0;
    currentProfile = 0;

    inputMesh->request_face_normals();
    inputMesh->request_vertex_normals();

    OpenMesh::IO::Options ropt;
    ropt += OpenMesh::IO::Options::VertexNormal;
    if (OpenMesh::IO::read_mesh(*inputMesh, fileName.toLocal8Bit().constData()),
            ropt)
    {
        inputMesh->update_face_normals();
        inputMesh->update_vertex_normals();
    }


    FloorPlanAndProfileExtractor extractor;

    extractor.extract(inputMesh, floorPlan, currentProfile, floorPlanSize);

    emit newFloorPlan();

    delete inputMesh;
    inputMesh = 0;

}

FloorVertex* Mesh::getFloorPlan()
{
    return floorPlan;
}

void Mesh::setUpdateOnMesh()
{
    updateOnMesh = true;
}

void Mesh::setLongUpdateOnMesh(bool b) {
    longUpdateOnMesh = b;
}


