/*
/*
Author: Kamya Hari
Class: ECE 6122 A
Last date Modified: 12/04/2024

Objective:
Game of Chess common file. Adapted from Lab3 solution
*/

#ifndef COMMON_H
#define COMMON_H

#include <unordered_map>
// Include GLM
#include <glm/glm.hpp>

// Mesh properties has table
typedef struct 
{
    bool hasBones;
    bool hasFaces;
    bool hasNormals;
    bool hasPositions;
    bool hasTangentsAndBitangents;
    bool hasTextureCoords;
    bool hasVertexColors;
    unsigned int numOfUVChannels;
} meshPropsT;

// Structure to hold target
// model matrix generation
typedef struct
{
    unsigned int rCnt;
    unsigned int rDis;
    float rAngle;
    glm::vec3 rAxis;
    glm::vec3 cScale;
    glm::vec3 tPos;
} tPosition;

// Chess board scaling
const float CBSCALE = 0.6f;
// Chess board square box size (per side)
//const float CHESS_BOX_SIZE = 3.f;
const float CHESS_BOX_SIZE = (float)(CBSCALE * 5.4);
// Chess pieces scaling
const float CPSCALE = 0.015f;
// Platform height
const float PHEIGHT = -3.0f;
// Hash to hold the target Model matrix spec for each Chess component
typedef std::unordered_map <std::string, tPosition> tModelMap;

#endif
