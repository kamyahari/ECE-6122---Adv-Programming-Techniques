/*
Author: Kamya Hari
Class: ECE 6122 A
Last date Modified: 12/04/2024

Description:
Header file for chessAnimation.cpp
*/
#pragma once

#include <glm/glm.hpp>
#include <queue>
#include <string>
#include <map>
#include <set>
#include <tuple>
#include "chessCommon.h"

enum class AnimationPhase {
    NONE,
    LIFT,
    MOVE,
    LOWER,
    CAPTURE_MOVE,
    COMPLETE
};

class PieceAnimation {
public:
    PieceAnimation();
    void startAnimation(const std::string& piece, const glm::vec3& start, const glm::vec3& end, bool isCapturing = false);
    void startCaptureAnimation(const std::string& piece, const glm::vec3& start, const glm::vec3& capturePos);
    glm::vec3 getCurrentPosition(float deltaTime);
    bool isComplete() const;

    std::string pieceId;
    bool isAnimating;
    bool isCapture;
    bool waitForCapture;

private:
    float easeInOutCubic(float t);

    AnimationPhase currentPhase;
    glm::vec3 startPos;
    glm::vec3 endPos;
    glm::vec3 captureDestination;
    float currentTime;
    float liftHeight;
    float liftDuration;
    float moveDuration;
};

// Define MoveInfo structure in the header
struct MoveInfo {
    std::string pieceId;
    glm::vec3 startPos;
    glm::vec3 endPos;
    std::string capturedPieceId;
    glm::vec3 capturedPiecePos;
    bool isWhite;
};

class ChessAnimationManager {
public:
    ChessAnimationManager();

    void queueMove(const std::string& pieceId, const glm::vec3& start, const glm::vec3& end);
    void queueCaptureMove(
        const std::string& pieceId,
        const glm::vec3& start,
        const glm::vec3& end,
        const std::string& capturedPieceId,
        const glm::vec3& capturedPiecePos,
        bool capturedPieceIsWhite);
    void removePiece(const std::string& pieceId, const glm::vec3& currentPos, bool isWhite);
    void update(float deltaTime, tModelMap& modelMap);
    bool isAnimating() const;
    void clear();

    std::set<std::string> removedPieces;

private:
    PieceAnimation currentAnimation;
    std::map<std::string, PieceAnimation> captureAnimations;
    std::queue<MoveInfo> pendingMoves;
    std::queue<std::tuple<std::string, glm::vec3, bool>> pendingCapture;
    int whiteCaptureCount;
    int blackCaptureCount;
};