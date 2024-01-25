#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <unordered_map>
using namespace  std;

struct Vertex
{
	float a, b, c;
	int index;
    vector<int> relatedFaces;
    vector<int> relatedHalfEgesEndPoint;
};

struct Face
{
    int a, b, c, d;
    vector<int> edges;
    int centerPoint;
    int faceIndex;

};

struct HalfEdge
{
    int index;
    int start, end;
    int relatedFace;
    int pairEdgeRelatedFace;
    int pairEdgeIndex; 
    int edgePoint;
};

class ModelReader
{
public:
	string path;
	vector<Vertex> vertexList;
	vector<int> vertexIndices;
    vector<HalfEdge> halfEdges;
    vector<Face> faces;
	int faceNum;
    ModelReader();
	ModelReader(string filepath);
	void Load();
	void PrintVertex();
    void PrintVertexIndices();
    void PrintHalfedges();
	void Clear();
	void ApplyTransformation(const glm::mat4& transformationMatrix);
    void SubdivideTriangles();
    void catmullClarkSubdivision();
    void AddVertex(float a, float b, float c);
    void AddFace(int a, int b, int c, int d);
    void AddVertexIndex(int index) {
        vertexIndices.push_back(index);
    }
private:
    glm::vec3 minBounds;
    glm::vec3 maxBounds;

    Vertex Normalize(const Vertex& v)
    {
        float length = sqrt(v.a * v.a + v.b * v.b + v.c * v.c);
        return { v.a / length, v.b / length, v.c / length, v.index };
    }

    void CenterAndScaleModel()
    {
        minBounds = glm::vec3(std::numeric_limits<float>::max());
        maxBounds = glm::vec3(std::numeric_limits<float>::lowest());
        for (const Vertex& vertex : vertexList)
        {
            glm::vec3 vertexPos(vertex.a, vertex.b, vertex.c);
            minBounds = glm::min(minBounds, vertexPos);
            maxBounds = glm::max(maxBounds, vertexPos);
        }

        glm::vec3 modelCenter = 0.5f * (minBounds + maxBounds);

        glm::vec3 screenCenter(0.0f, 0.0f, 5.0f);

        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), screenCenter - modelCenter);

        float scaleFactor = 1.0f / std::max(std::max(maxBounds.x - minBounds.x, maxBounds.y - minBounds.y), maxBounds.z - minBounds.z);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor));

        ApplyTransformation(scaleMatrix);
    }

    bool IsPointInsideModel(const Vertex& point)
    {
        // Implement a method to check if the point is inside the original model
        // This could involve using a ray-casting algorithm or other techniques
        // Return true if inside, false otherwise
        // Example: simple bounding box check
        return (
            point.a >= minBounds.x && point.a <= maxBounds.x &&
            point.b >= minBounds.y && point.b <= maxBounds.y &&
            point.c >= minBounds.z && point.c <= maxBounds.z);
    }

    bool campare(Vertex a, Vertex b)
    {
        if (a.a == b.a && a.b == b.b && a.c == b.c) {
            return true;
        }

        return false;

    }

    bool existOrNot(Vertex vertex, Face face) {

        Vertex a = vertexList[face.a];
        Vertex b = vertexList[face.b];
        Vertex c = vertexList[face.c];
        Vertex d = vertexList[face.d];
       
        if (campare(vertex, a))
            return true;

        if (campare(vertex, b))
            return true;

        if (campare(vertex, c))
            return true;

        if (campare(vertex, d))
            return true;

        return false;
    }

    void addNewVertexToVertexList(Vertex &vertex) {
        bool vertexExists = false;
        // Check if the vertex already exists in the vertexList
        for (int j = 0; j < vertexList.size(); ++j) {
            if (campare(vertex, vertexList[j])) {
                vertexExists = true;
                vertex.index = vertexList[j].index;
                break;
            }
        }
        
        if (vertexExists == false) {
            vertex.index = vertexList.size();
            vertexList.push_back(vertex);
        }
    }

    int checkHalfedges() {

        int count = 0;
        for (int i = 0; i < halfEdges.size(); i++) {
            for (int j = 0; j < halfEdges.size(); j++) {
                if (i != j && halfEdges[i].start == halfEdges[j].start && halfEdges[i].end == halfEdges[j].end) {
                    count += 1;
                    break;
                }
            }
        }
        return count;
    }


    int returnTheBiggest(const std::vector<int>& list) {
        std::unordered_map<int, int> countMap; // 用于记录数字及其出现次数

        // 遍历列表，统计数字出现次数
        for (int num : list) {
            countMap[num]++;
        }

        int maxNum = 0; // 最大重复次数的数字
        int maxCount = 0; // 最大重复次数

        // 遍历计数映射，找到重复次数最多的数字
        for (const auto& entry : countMap) {
            if (entry.second > maxCount) {
                maxNum = entry.first;
                maxCount = entry.second;
            }
        }

        return maxCount;
    }
};

