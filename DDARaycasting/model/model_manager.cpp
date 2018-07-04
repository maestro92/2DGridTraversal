#include "model_manager.h"

/*

when working with models in blender
the -z axis in game is the y axis in blender, 

so to be sure to orient the models in the right direction


*/

ModelManager::ModelManager()
{

}

ModelManager::~ModelManager()
{

}

void ModelManager::init()
{
	m_quad = new QuadModel(1, 1);
	m_centeredQuad = new QuadModel(-0.5, 0.5, -0.5, 0.5);
	m_xyzAxis = new XYZAxisModel();

	m_models.resize(ModelEnum::NUM_MODELS);
	m_models[ModelEnum::quad] = m_quad;
	m_models[ModelEnum::centeredQuad] = m_centeredQuad;
	m_models[ModelEnum::xyzAxis] = m_xyzAxis;
}


void ModelManager::enableVertexAttribArrays()
{
	glEnableVertexAttribArray(POSITION_VERTEX_ATTRIB);
	glEnableVertexAttribArray(NORMAL_VERTEX_ATTRIB);
	glEnableVertexAttribArray(COLOR_VERTEX_ATTRIB);
	glEnableVertexAttribArray(UV_VERTEX_ATTRIB);
	glEnableVertexAttribArray(BONE_IDS_ATTRIB);
	glEnableVertexAttribArray(BONE_WEIGHTS_ATTRIB);
}



void ModelManager::disableVertexAttribArrays()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableVertexAttribArray(POSITION_VERTEX_ATTRIB);
	glDisableVertexAttribArray(NORMAL_VERTEX_ATTRIB);
	glDisableVertexAttribArray(COLOR_VERTEX_ATTRIB);
	glDisableVertexAttribArray(UV_VERTEX_ATTRIB);
	glDisableVertexAttribArray(BONE_IDS_ATTRIB);
	glDisableVertexAttribArray(BONE_WEIGHTS_ATTRIB);
}



void ModelManager::buildLinesBetweenTwoPoints2D(glm::vec2 p0, glm::vec2 p1, float thickness,
	vector<VertexData>& vertices,
	vector<unsigned int>& indices)
{
	glm::vec3 pos0 = glm::vec3(p0.x, p0.y, 0);
	glm::vec3 pos1 = glm::vec3(p1.x, p1.y, 0);
	buildLinesBetweenTwoPoints2D(pos0, pos1, thickness, vertices, indices);
}



void ModelManager::buildLinesBetweenTwoPoints2D(glm::vec3 p0, glm::vec3 p1, float thickness,
	vector<VertexData>& vertices,
	vector<unsigned int>& indices)
{
	VertexData tmp;

	glm::vec3 dir = p1 - p0;
	dir = glm::normalize(dir);

	glm::vec3 Z_AXIS = glm::vec3(0.0f, 0.0f, -1.0f);

	glm::vec3 normal = glm::cross(dir, Z_AXIS);
	normal = glm::normalize(normal);


	glm::vec3 pos0 = p1 - normal * thickness;
	glm::vec3 pos1 = p0 - normal * thickness;
	glm::vec3 pos2 = p1 + normal * thickness;
	glm::vec3 pos3 = p0 + normal * thickness;
	
	int indicesStart = vertices.size();

	/// 0. bot left
	tmp.position = pos0;
	vertices.push_back(tmp);

	/// 1. bot right
	tmp.position = pos1;
	vertices.push_back(tmp);

	/// 2. top right
	tmp.position = pos2;
	vertices.push_back(tmp);

	/// 3. top left
	tmp.position = pos3;
	vertices.push_back(tmp);

	indices.push_back(indicesStart + 1);
	indices.push_back(indicesStart + 2);
	indices.push_back(indicesStart + 0);

	indices.push_back(indicesStart + 1);
	indices.push_back(indicesStart + 3);
	indices.push_back(indicesStart + 2);
}




void ModelManager::shutDown()
{


}

Model* ModelManager::get(int modelEnum)
{
	return m_models[modelEnum];
}

// http://strike-counter.com/cs-go-stats/weapons-stats
