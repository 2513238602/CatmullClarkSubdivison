#include "ModelReader.h"
#include <glm/fwd.hpp>
#include <map>

ModelReader::ModelReader()
{
}

ModelReader::ModelReader(string filepath)
{
    path = filepath;
}

void ModelReader::Load()
{
    ifstream model(path);
    if (!model.is_open()) {
        return;
    }

    string line;
    getline(model, line);
    faceNum = std::stoi(line);

    // Temporary storage for vertices
    std::vector<Vertex> tempVertices;

    // Read vertex data
    while (getline(model, line)) {
        istringstream iss(line);
        Vertex vertex;
        iss >> vertex.a >> vertex.b >> vertex.c;
        tempVertices.push_back(vertex);
    }

    // Clear existing data
    vertexIndices.clear();
    vertexList.clear();
    faces.clear();
    halfEdges.clear();

    // Iterate through the temporary vertices and build half-edge data structure
    for (int i = 0; i < tempVertices.size(); ++i) {
        bool vertexExists = false;

        // Check if the vertex already exists in the vertexList
        for (int j = 0; j < vertexList.size(); ++j) {
            if (campare(tempVertices[i], vertexList[j])) {
                vertexExists = true;
                vertexIndices.push_back(vertexList[j].index);
                break;
            }
        }

        // If the vertex doesn't exist, add it to vertexList and update index
        if (!vertexExists) {
            Vertex newVertex = tempVertices[i];
            newVertex.index = vertexList.size();
            vertexList.push_back(newVertex);
            vertexIndices.push_back(newVertex.index);
        }
    }
    //PrintVertexIndices();
    CenterAndScaleModel();
}

void ModelReader::SubdivideTriangles()
{        
    // Create a copy of the current vertices and indices
    std::vector<int> originalIndices = vertexIndices;

    // Clear the existing data
    vertexIndices.clear();

    faceNum = 0;
   
    // Iterate through each original triangle and subdivide
    for (int i = 0; i < originalIndices.size(); i += 3)
    {
        // Get the vertices of the current triangle
        Vertex v0 = vertexList[originalIndices[i]];
        Vertex v1 = vertexList[originalIndices[i + 1]];
        Vertex v2 = vertexList[originalIndices[i + 2]];

        // Calculate midpoints of edges
        Vertex mid01, mid12, mid20;
        mid01.a = (v0.a + v1.a) / 2.0f;
        mid01.b = (v0.b + v1.b) / 2.0f;
        mid01.c = (v0.c + v1.c) / 2.0f;

        mid12.a = (v1.a + v2.a) / 2.0f;
        mid12.b = (v1.b + v2.b) / 2.0f;
        mid12.c = (v1.c + v2.c) / 2.0f;

        mid20.a = (v2.a + v0.a) / 2.0f;
        mid20.b = (v2.b + v0.b) / 2.0f;
        mid20.c = (v2.c + v0.c) / 2.0f;

        // Calculate the center point of the triangle
        Vertex center;
        center.a = (v0.a + v1.a + v2.a) / 3.0f;
        center.b = (v0.b + v1.b + v2.b) / 3.0f;
        center.c = (v0.c + v1.c + v2.c) / 3.0f;

        // Add the subdivided vertices
       
        addNewVertexToVertexList(mid01);
        addNewVertexToVertexList(mid12);
        addNewVertexToVertexList(mid20);
        addNewVertexToVertexList(center);

        int mid01Index = mid01.index;
        int mid20Index = mid20.index;
        int mid12Index = mid12.index;
        int centerIndex = center.index;

        // Update the original indices to form the three new quadrilaterals
        Face one, two, three;
        vertexIndices.push_back(originalIndices[i]);
        vertexIndices.push_back(mid01Index);
        vertexIndices.push_back(centerIndex);
        vertexIndices.push_back(mid20Index);
        one.a = originalIndices[i];
        one.b = mid01Index;
        one.c = centerIndex;
        one.d = mid20Index ;
        
        vertexIndices.push_back(originalIndices[i + 1]);
        vertexIndices.push_back(mid12Index);
        vertexIndices.push_back(centerIndex);
        vertexIndices.push_back(mid01Index);
        two.a = originalIndices[i + 1];
        two.b = mid12Index;
        two.c = centerIndex;
        two.d = mid01Index ;

        vertexIndices.push_back(originalIndices[i + 2]);
        vertexIndices.push_back(mid20Index);
        vertexIndices.push_back(centerIndex);
        vertexIndices.push_back(mid12Index);
        three.a = originalIndices[i + 2];
        three.b = mid20Index;
        three.c = centerIndex ;
        three.d = mid12Index ;

        faces.push_back(one);
        faces.push_back(two);
        faces.push_back(three);
    }
}

void ModelReader::catmullClarkSubdivision()
{
    //get the number of original vertices
    int originalVerticesNumber = vertexList.size();

    //create halfedges and update faces
    for (int i = 0; i < faces.size(); i++) {
        //faces[i].faceIndex = i;

        Vertex a = vertexList[faces[i].a];
        Vertex b = vertexList[faces[i].b];
        Vertex c = vertexList[faces[i].c];
        Vertex d = vertexList[faces[i].d];

        Vertex center;
        center.a = (a.a + b.a + c.a + d.a) / 4.0f;
        center.b = (a.b + b.b + c.b + d.b) / 4.0f;
        center.c = (a.c + b.c + c.c + d.c) / 4.0f;

        addNewVertexToVertexList(center);

        //update the face centerpoint
        faces[i].centerPoint = center.index;

        HalfEdge one, two, three, four;
        one.start = a.index;
        one.end = b.index;

        two.start = b.index;
        two.end = c.index;

        three.start = c.index;
        three.end = d.index;

        four.start = d.index;
        four.end = a.index;

        one.relatedFace = i;        
        two.relatedFace = i;
        three.relatedFace = i;
        four.relatedFace = i;

        one.index = halfEdges.size();
        halfEdges.push_back(one);
        two.index = halfEdges.size();
        halfEdges.push_back(two);
        three.index = halfEdges.size();
        halfEdges.push_back(three);
        four.index = halfEdges.size();
        halfEdges.push_back(four);

        faces[i].edges.push_back(one.index);
        faces[i].edges.push_back(two.index);
        faces[i].edges.push_back(three.index);
        faces[i].edges.push_back(four.index);
    }

    int count = 0;
    //add halfedgeindex to every halfedge
    for (int i = 0; i < halfEdges.size(); i++) {       
        for (int j = 0; j < halfEdges.size(); j++) {
            if (i != j && halfEdges[i].start == halfEdges[j].end && halfEdges[i].end == halfEdges[j].start) {
                halfEdges[i].pairEdgeIndex = j;
                halfEdges[i].pairEdgeRelatedFace = halfEdges[j].relatedFace;
                break;
            }
        }

        if (halfEdges[i].pairEdgeIndex < 0)
            count += 1;
    }

    //get related face to every original vertices
    for (int i = 0; i < originalVerticesNumber; i++) {
        vertexList[i].relatedFaces.clear();
        for (int j = 0; j < faces.size(); j++) {
            if (existOrNot(vertexList[i], faces[j])) {
                vertexList[i].relatedFaces.push_back(j);
            }
        }
    }

    //get related halfeges to every original vertices
    for (int i = 0; i < originalVerticesNumber; i++) {
        //vertexList[i].relatedHalfEgesEndPoint.clear();
        for (int j = 0; j < halfEdges.size(); j++) {
            if (campare(vertexList[i], vertexList[halfEdges[j].start])) {
                vertexList[i].relatedHalfEgesEndPoint.push_back(halfEdges[j].end);
            }
        }
    }
    
    vector<int> testList;
    //calculate edge point to every halfedge
    for (int i = 0; i < halfEdges.size(); i++) {
        Vertex start = vertexList[halfEdges[i].start];
        Vertex end = vertexList[halfEdges[i].end];

        //calculate edge center point
        Vertex edgeCenterPoint;

        edgeCenterPoint.a = (start.a + end.a) / 2.0f;
        edgeCenterPoint.b = (start.b + end.b) / 2.0f;
        edgeCenterPoint.c = (start.c + end.c) / 2.0f;

        //calculate related faces center point
        Vertex relatedFacesCenterPoint;
        Face a = faces[halfEdges[i].relatedFace];
        Face b = faces[halfEdges[i].pairEdgeRelatedFace];

        relatedFacesCenterPoint.a = (vertexList[a.centerPoint].a + vertexList[b.centerPoint].a) / 2.0f;
        relatedFacesCenterPoint.b = (vertexList[a.centerPoint].b + vertexList[b.centerPoint].b) / 2.0f;
        relatedFacesCenterPoint.c = (vertexList[a.centerPoint].c + vertexList[b.centerPoint].c) / 2.0f;

        if (halfEdges[i].pairEdgeRelatedFace < 0) {
            relatedFacesCenterPoint = vertexList[a.centerPoint];
            cout << "kongkogn " << endl;
        }
        //calculate edge point
        Vertex edgePoint;
        edgePoint.a = (edgeCenterPoint.a + relatedFacesCenterPoint.a) / 2.0f;
        edgePoint.b = (edgeCenterPoint.b + relatedFacesCenterPoint.b) / 2.0f;
        edgePoint.c = (edgeCenterPoint.c + relatedFacesCenterPoint.c) / 2.0f;

        addNewVertexToVertexList(edgePoint);
        halfEdges[i].edgePoint = edgePoint.index;
    }


    //update original vertices
    for (int i = 0; i < originalVerticesNumber; i++) {
        Vertex averageCenterFacePoint;
        
        averageCenterFacePoint.a = 0.0f;
        averageCenterFacePoint.b = 0.0f;
        averageCenterFacePoint.c = 0.0f;
        for (int j = 0; j < vertexList[i].relatedFaces.size(); j++) {
            averageCenterFacePoint.a += vertexList[faces[vertexList[i].relatedFaces[j]].centerPoint].a;
            averageCenterFacePoint.b += vertexList[faces[vertexList[i].relatedFaces[j]].centerPoint].b;
            averageCenterFacePoint.c += vertexList[faces[vertexList[i].relatedFaces[j]].centerPoint].c;
        }

        averageCenterFacePoint.a = averageCenterFacePoint.a / float(vertexList[i].relatedFaces.size());
        averageCenterFacePoint.b = averageCenterFacePoint.b / float(vertexList[i].relatedFaces.size());
        averageCenterFacePoint.c = averageCenterFacePoint.c / float(vertexList[i].relatedFaces.size());

        Vertex averageCenterEdgePoint;
        averageCenterEdgePoint.a = 0.0f;
        averageCenterEdgePoint.b = 0.0f;
        averageCenterEdgePoint.c = 0.0f;
        for (int j = 0; j < vertexList[i].relatedHalfEgesEndPoint.size(); j++) {
            averageCenterEdgePoint.a += vertexList[vertexList[i].relatedHalfEgesEndPoint[j]].a;
            averageCenterEdgePoint.b += vertexList[vertexList[i].relatedHalfEgesEndPoint[j]].b;
            averageCenterEdgePoint.c += vertexList[vertexList[i].relatedHalfEgesEndPoint[j]].c;
        }
        averageCenterEdgePoint.a = averageCenterEdgePoint.a / float(vertexList[i].relatedHalfEgesEndPoint.size());
        averageCenterEdgePoint.b = averageCenterEdgePoint.b / float(vertexList[i].relatedHalfEgesEndPoint.size());
        averageCenterEdgePoint.c = averageCenterEdgePoint.c / float(vertexList[i].relatedHalfEgesEndPoint.size());

        Vertex newVertex;
        float faceNumber = vertexList[i].relatedFaces.size();
        float paraOne = float(1.0f / 4.0f);
        float paraTwo = float(2.0f / 4.0f);
        newVertex.a = paraOne * vertexList[i].a + paraOne * averageCenterFacePoint.a + paraTwo * averageCenterEdgePoint.a;
        newVertex.b = paraOne * vertexList[i].b + paraOne * averageCenterFacePoint.b + paraTwo * averageCenterEdgePoint.b;
        newVertex.c = paraOne * vertexList[i].c + paraOne * averageCenterFacePoint.c + paraTwo * averageCenterEdgePoint.c;
        
        newVertex.index = i;
        vertexList[i] = newVertex;
        //cout << i<<"  "<<newVertex.a << "  " << newVertex.b << "  " << newVertex.c << endl;
    }

    vector<Face> newFaces;
    vector<int> newVertexIndices;

    //PrintVertex();

    for (int i = 0; i < faces.size(); i++) {
        Face face = faces[i];

        int i0,i1, i2, i3, i4, i5, i6, i7, i8;
        i0 = vertexList[face.a].index;
        i1 = vertexList[face.b].index;
        i2 = vertexList[face.c].index;
        i3 = vertexList[face.d].index;
        i4 = vertexList[halfEdges[face.edges[0]].edgePoint].index;
        i5 = vertexList[halfEdges[face.edges[1]].edgePoint].index;
        i6 = vertexList[halfEdges[face.edges[2]].edgePoint].index;
        i7 = vertexList[halfEdges[face.edges[3]].edgePoint].index;
        i8 = vertexList[face.centerPoint].index;

        Face one, two, three, four;

        one.a = i0;
        one.b = i4;
        one.c = i8;
        one.d = i7;
        one.faceIndex = newFaces.size();
        newFaces.push_back(one);
        newVertexIndices.push_back(i0);
        newVertexIndices.push_back(i4);
        newVertexIndices.push_back(i8);
        newVertexIndices.push_back(i7);

        two.a = i1;
        two.b = i5;
        two.c = i8;
        two.d = i4;
        two.faceIndex = newFaces.size();
        newFaces.push_back(two);
        newVertexIndices.push_back(i1);
        newVertexIndices.push_back(i5);
        newVertexIndices.push_back(i8);
        newVertexIndices.push_back(i4);

        three.a = i2;
        three.b = i6;
        three.c = i8;
        three.d = i5;
        three.faceIndex = newFaces.size();
        newFaces.push_back(three);
        newVertexIndices.push_back(i2);
        newVertexIndices.push_back(i6);
        newVertexIndices.push_back(i8);
        newVertexIndices.push_back(i5);

        four.a = i3;
        four.b = i7;
        four.c = i8;
        four.d = i6;
        four.faceIndex = newFaces.size();
        newFaces.push_back(four);
        newVertexIndices.push_back(i3);
        newVertexIndices.push_back(i7);
        newVertexIndices.push_back(i8);
        newVertexIndices.push_back(i6);
        
    }

    vertexIndices = newVertexIndices;
    //PrintVertexIndices();
    faces = newFaces;      
    halfEdges.clear();
    //cout << vertexList.size() << endl;
    //cout << faces.size() << endl;
}



void ModelReader::ApplyTransformation(const glm::mat4& transformationMatrix)
{
    // Iterate through each vertex and apply the transformation
    for (Vertex& vertex : vertexList)
    {
        // Convert the vertex position to a glm::vec4 for transformation
        glm::vec4 vertexPosition(vertex.a, vertex.b, vertex.c, 1.0f);

        // Apply the transformation
        glm::vec4 transformedPosition = transformationMatrix * vertexPosition;

        // Update the vertex position with the transformed values
        vertex.a = transformedPosition.x;
        vertex.b = transformedPosition.y;
        vertex.c = transformedPosition.z;
    }

    // Note: This assumes that your indices and vertices are correctly linked.
    // If your model structure is more complex, you may need to adapt this logic accordingly.
}

void ModelReader::AddVertex(float a, float b, float c) {
    Vertex vertex;
    vertex.a = a;
    vertex.b = b;
    vertex.c = c;
    vertex.index = vertexList.size();
    vertexList.push_back(vertex);
}

void ModelReader::AddFace(int a, int b, int c, int d) {
    Face face;
    face.a = a;
    face.b = b;
    face.c = c;
    face.d = d;
    face.faceIndex = faces.size();
    faces.push_back(face);
}


void ModelReader::PrintHalfedges()
{

    std::cout << "Half-Edge Information:" << std::endl;

    for (int i = 0; i < halfEdges.size(); ++i)
    {
        const HalfEdge& edge = halfEdges[i];

        std::cout << "Half-Edge " << i << ":" << edge.start << "  " << edge.end << std::endl;

        std::cout << std::endl;
    }

}



void ModelReader::PrintVertex()
{
    std::cout << "Vertices Information:" << std::endl;
    for (int i = 0; i < vertexList.size(); i++) {
        cout << vertexList[i].index << " " << vertexList[i].a << " " << vertexList[i].b <<
            " " << vertexList[i].c << endl;
    }
}

void ModelReader::Clear()
{
    vertexIndices.clear();
    vertexList.clear();
    faceNum = 0;
}

void ModelReader::PrintVertexIndices()
{
    std::cout << "VertexIndices Information:" << std::endl;
    for (int i = 0; i < vertexIndices.size(); i += 4) {
        cout << i/4 <<"    "<< vertexIndices[i] << " " << vertexIndices[i + 1] << " " << vertexIndices[i + 2] << "  " << vertexIndices[i + 3] << endl;
    }
}
