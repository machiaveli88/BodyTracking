#pragma once

#include "OpenGEX/OpenGEX.h"

#include <Kore/Graphics/Graphics.h>
#include <Kore/Graphics/Texture.h>

#include <vector>

struct Mesh {
	int numFaces;
	int numVertices;
	int numUVs;
	int numNormals;
	
	float* vertices;
	int* indices;
	float* normals;
	float* texcoord;
};

class MeshObject {

public:
	MeshObject(const char* meshFile, const char* textureFile, const Kore::VertexStructure& structure, float scale = 1.0f);
	void render(Kore::TextureUnit tex);
    
    Kore::mat4 M;
	
	
private:
	Kore::VertexBuffer* vertexBuffer;
	Kore::IndexBuffer* indexBuffer;
	
	Kore::Texture* image;
    std::vector<Mesh*> meshes;
	
	void LoadObj(const char* filename);
	void ConvertObjectStructure(const Structure& structure);
	Mesh* ConvertGeometryObject(const OGEX::GeometryObjectStructure& structure);
	Mesh* ConvertMesh(const OGEX::MeshStructure& structure);
	
};
