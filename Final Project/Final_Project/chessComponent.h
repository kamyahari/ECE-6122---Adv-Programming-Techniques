/*
*
Author: Kamya Hari
Class: ECE 6122 A
Last Date Modified: 12/04/2024
Objective:

Chess component class header file. Adapted from Lab3 solution
*/

#ifndef CHESS_COMPONENTS_H
#define CHESS_COMPONENTS_H

#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include "chessCommon.h"

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Include GLEW
#include <GL/glew.h>

// Load BMP function support
#include <common/texture.hpp>

class chessComponent
{
private:
    // Properties of a Chess component
    // mesh
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    // OpenGL Buffers management
    GLuint vertexbuffer = 0;
    GLuint uvbuffer = 0;
    GLuint normalbuffer = 0;
    GLuint elementbuffer = 0;

    // Component ID
    std::string cName;
    std::string cTextureFile;

    // Mesh properties
    meshPropsT meshProps;
    glm::vec3 cGeometricCener = { 0, 0, 0 };
    glm::vec3 cBoundingLimitsMin = { 0, 0, 0 };
    glm::vec3 cBoundingLimitsMax = { 0, 0, 0 };

    // Texture properties
    GLuint Texture;

    // Compute the Geometric center
    // Inputs: None
    // Output: None
    void getGeometricCenter();

    // Compute the Bounding box
    // Inputs: None
    // Output: None
    void getBoundingBox();


public:
    // Constructor function
    chessComponent();
    // destructor function
    ~chessComponent();
    // Reserve storage
    // Inputs: memory limits for performance
    // Output: None
    void reserveStorage(const unsigned int & vCapacity, const unsigned int & fCapacity);
    // Add vertices
    // Inputs: Vertices read from OBJ file
    // Output: None
    void addVertices(glm::vec3 & objVertice);
    // Add Texture Coordinates
    // Inputs: Texture coordinates read from OBJ file
    // Output: None
    void addTextureCor(glm::vec3 & objUVW);
    // Add Vertices Normals
    // Inputs: Vertices read from OBJ file
    // Output: None
    void addVerNormals(glm::vec3& objVerNormal);
    // Add Face indices
    // Inputs: Face vertices read from OBJ file
    // Output: None
    void addFaceIndices(unsigned int *objFaceIndice);
    // Setup rendering buffers
    // Inputs: None
    // Output: None
    void setupGLBuffers();
    // Setup Texture buffers
    // Inputs: None
    // Output: None
    void setupTextureBuffers();
    // Setup rendering buffers
    // Inputs: None
    // Output: None
    void setupTexture(GLuint & TextureID) const;
    // Render a mesh
    // Inputs: None
    // Output: None
    void renderMesh() const;
    // Render a mesh
    // Inputs: None
    // Output: None
    void deleteGLBuffers();
    // Stores a component ID
    // Inputs: None
    // Output: None
    void storeComponentID(std::string cName);
    // Stores a Texture file name
    // Inputs: None
    // Output: None
    void storeTextureID(std::string cTextureFile);
    // Store Mesh properties (mainly for debug and bound checks)
    // Inputs: None
    // Output: None
    void storeMeshProps(meshPropsT meshProps);
    // Generate model matrix
    // Inputs: None
    // Output: None
    glm::mat4 genModelMatrix(const tPosition & cTPosition) const;
    // Get ID
    // Inputs: None
    // Output: ID
    std::string getComponentID() const;
};

#endif
