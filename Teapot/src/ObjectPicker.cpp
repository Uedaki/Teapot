#include "ObjectPicker.h"

#include <glm/gtx/intersect.hpp>

#include <set>
#include <utility>

#include "Application.h"

teapot::ObjectPicker::Result teapot::ObjectPicker::findVertex(glm::vec3 origin, glm::vec3 dir, float maxDist)
{
	Result result;
	result.mesh = &(Application::get().getCollection().mesh);

	float lowestDist = -1;
	for (uint32_t i = 0; i < result.mesh->getVertices().size(); i++)
	{
		glm::vec3 rLoc = result.mesh->getVertices()[i].pos;
		glm::vec4 loc = result.mesh->getTransform() * glm::vec4(rLoc.x, rLoc.y, rLoc.z, 1);

		float dist = glm::length(origin - glm::vec3(loc));
		if (dist < lowestDist || lowestDist == -1)
		{
			lowestDist = dist;
			result.v1 = i;
		}
	}

	if (lowestDist < 0.1)
		return (result);
	else if (lowestDist > maxDist)
		return (Result());
	else
		return (findVertex(origin + dir * lowestDist, dir, maxDist));
}

teapot::ObjectPicker::Result teapot::ObjectPicker::findEdge(glm::vec3 origin, glm::vec3 dir, float maxDist)
{
	Result result;
	result.mesh = &(Application::get().getCollection().mesh);

	float lowestDist = -1;
	const std::vector<vk::Vertex> &vertices = result.mesh->getVertices();
	const std::vector<vk::Indice> &indices = result.mesh->getIndices();

	std::set<std::pair<uint32_t, uint32_t>> edges;
	for (uint32_t i = 0; i < indices.size(); i++)
	{
		uint32_t i1 = indices[i].a;
		uint32_t i2 = indices[i].b;
		uint32_t i3 = indices[i].c;

		edges.emplace(std::min(i1, i2), std::max(i1, i2));
		edges.emplace(std::min(i2, i3), std::max(i2, i3));
		edges.emplace(std::min(i3, i1), std::max(i3, i1));
	}

	for (auto &edge : edges)
	{
		glm::vec4 v1 = glm::vec4(vertices[edge.first].pos, 1) * result.mesh->getTransform();
		glm::vec4 v2 = glm::vec4(vertices[edge.second].pos, 1) * result.mesh->getTransform();
		glm::vec3 e1 = v2 - v1;
		glm::vec3 e2 = origin - glm::vec3(v1);
		glm::vec3 e3 = origin - glm::vec3(v2);
		float dot1 = glm::dot(e1, e2);
		float dot2 = glm::dot(-e1, e3);

		float dist = maxDist;
		if (dot1 < 0)
			dist = glm::length(e2);
		else if (dot2 < 0)
			dist = glm::length(e3);
		else
			dist = glm::length(glm::cross(e1, e2)) / glm::length(e1);
		
		if ((dist < lowestDist || lowestDist == -1))
		{
			lowestDist = dist;
			result.v1 = edge.first;
			result.v2 = edge.second;
		}
	}

	if (lowestDist < 0.1)
		return (result);
	else if (lowestDist > maxDist)
		return (Result());
	else
		return (findEdge(origin + dir * lowestDist, dir, maxDist));
}

teapot::ObjectPicker::Result teapot::ObjectPicker::findFace(glm::vec3 origin, glm::vec3 dir, float maxDist)
{
	Result result;
	result.mesh = &(Application::get().getCollection().mesh);

	float lowestDist = -1;
	const std::vector<vk::Vertex> &vertices = result.mesh->getVertices();
	const std::vector<vk::Indice> &indices = result.mesh->getIndices();
	for (uint32_t i = 0; i < indices.size(); i++)
	{
		glm::vec4 v1 = glm::vec4(vertices[indices[i].a].pos, 1) * result.mesh->getTransform();
		glm::vec4 v2 = glm::vec4(vertices[indices[i].b].pos, 1) * result.mesh->getTransform();
		glm::vec4 v3 = glm::vec4(vertices[indices[i].c].pos, 1) * result.mesh->getTransform();

		float dist;
		glm::vec2 bary;
		if (glm::intersectRayTriangle(origin, dir, glm::vec3(v1), glm::vec3(v2), glm::vec3(v3), bary, dist))
		{
			if (dist < lowestDist || lowestDist == -1)
			{
				lowestDist = dist;
				result.v1 = indices[i].a;
				result.v2 = indices[i].b;
				result.v3 = indices[i].c;
			}
		}
	}

	if (lowestDist == -1)
		return (Result());
	else
		return (result);
}