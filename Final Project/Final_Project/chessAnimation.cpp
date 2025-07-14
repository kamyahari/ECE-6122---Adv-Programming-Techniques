/*
Author: Kamya Hari
Class: ECE 6122 A
Last date Modified: 12/04/2024

Description:
To create classes that can handle/manage the animation part of the chess game. has two classes - PieceAnimation that takes care of piece wise animation and a
Chess animation manager to overlook the overall process.
*/

#include "chessAnimation.h"
#include <algorithm>
#include <iostream>
#include <cmath>

//create a class to model every piece's animation
PieceAnimation::PieceAnimation()
    : currentPhase(AnimationPhase::NONE)
    , isAnimating(false)
    , currentTime(0.0f)
    , liftHeight(2.0f)
    , liftDuration(0.3f)
    , moveDuration(0.4f)
    , isCapture(false)
    , startPos(0.0f)
    , endPos(0.0f)
    , captureDestination(0.0f)
    , waitForCapture(false) {}

//for normal animation
void PieceAnimation::startAnimation(const std::string& piece, const glm::vec3& start, const glm::vec3& end, bool isCapturing) {
    pieceId = piece;
    startPos = start;
    endPos = end;
    currentTime = 0.0f;
    isAnimating = true;
    isCapture = false;
    waitForCapture = isCapturing;
    currentPhase = AnimationPhase::LIFT;
    //std::cout << "Starting move animation for " << piece << (isCapturing ? " (capturing move)" : "") << std::endl;
}

//for capture by the enemy
void PieceAnimation::startCaptureAnimation(const std::string& piece, const glm::vec3& start, const glm::vec3& capturePos) {
    pieceId = piece;
    startPos = start;
    captureDestination = capturePos;
    currentTime = 0.0f;
    isAnimating = true;
    isCapture = true;
    currentPhase = AnimationPhase::LIFT;
    //std::cout << "Starting capture animation for " << piece << std::endl;
}

float PieceAnimation::easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

//getting current position of the piece
glm::vec3 PieceAnimation::getCurrentPosition(float deltaTime) {
    if (!isAnimating) {
        return isCapture ? captureDestination : endPos;
    }

    currentTime += deltaTime;
    float phaseProgress = 0.0f;
    glm::vec3 currentPos;

    switch (currentPhase) {
    case AnimationPhase::LIFT: { //lifting in the air phase of the animation
        phaseProgress = std::min(currentTime / liftDuration, 1.0f);
        float heightOffset = liftHeight * easeInOutCubic(phaseProgress);
        currentPos = startPos + glm::vec3(0.0f, 0.0f, heightOffset);

        if (phaseProgress >= 1.0f) {
            currentPhase = isCapture ? AnimationPhase::CAPTURE_MOVE : AnimationPhase::MOVE;
            currentTime = 0.0f;
        }
        break;
    }
    
                             //move animation case
    case AnimationPhase::MOVE:
    case AnimationPhase::CAPTURE_MOVE: {
        phaseProgress = std::min(currentTime / moveDuration, 1.0f);
        glm::vec3 targetPos = (currentPhase == AnimationPhase::CAPTURE_MOVE) ? captureDestination : endPos;
        glm::vec3 startPosHigher = startPos + glm::vec3(0.0f, 0.0f, liftHeight);
        glm::vec3 endPosHigher = targetPos + glm::vec3(0.0f, 0.0f, liftHeight);
        currentPos = glm::mix(startPosHigher, endPosHigher, easeInOutCubic(phaseProgress));

        if (phaseProgress >= 1.0f) {
            currentPhase = AnimationPhase::LOWER;
            currentTime = 0.0f;
        }
        break;
    }
//lowers down on the destination
    case AnimationPhase::LOWER: {
        phaseProgress = std::min(currentTime / liftDuration, 1.0f);
        float heightOffset = liftHeight * (1.0f - easeInOutCubic(phaseProgress));
        glm::vec3 targetPos = isCapture ? captureDestination : endPos;
        currentPos = targetPos + glm::vec3(0.0f, 0.0f, heightOffset);

        if (phaseProgress >= 1.0f) {
            currentPhase = AnimationPhase::COMPLETE;
            isAnimating = false;
            currentPos = isCapture ? captureDestination : endPos;
        }
        break;
    }

    default:
        currentPos = isCapture ? captureDestination : endPos;
        break;
    }

    return currentPos;
}

//once it is done - flag is set
bool PieceAnimation::isComplete() const {
    return currentPhase == AnimationPhase::COMPLETE || !isAnimating;
}

//Another class to manage this animation across the chess Board
ChessAnimationManager::ChessAnimationManager()
    : whiteCaptureCount(0)
    , blackCaptureCount(0)
{
    removedPieces.clear();
}

//Queue an animation
void ChessAnimationManager::queueMove(const std::string& pieceId, const glm::vec3& start, const glm::vec3& end) {
    if (removedPieces.find(pieceId) == removedPieces.end()) {
        //std::cout << "Queueing move for piece: " << pieceId << std::endl;
        MoveInfo moveInfo{ pieceId, start, end, "", glm::vec3(0.0f), false };
        pendingMoves.push(moveInfo);
    }
}

//Queuing  -specifically when a piece is captured
void ChessAnimationManager::queueCaptureMove(
    const std::string& pieceId,
    const glm::vec3& start,
    const glm::vec3& end,
    const std::string& capturedPieceId,
    const glm::vec3& capturedPiecePos,
    bool capturedPieceIsWhite) {

    if (removedPieces.find(pieceId) == removedPieces.end()) {
        //std::cout << "Queueing capture move for piece: " << pieceId << std::endl;
        MoveInfo moveInfo{
            pieceId, start, end,
            capturedPieceId, capturedPiecePos,
            capturedPieceIsWhite
        };
        pendingMoves.push(moveInfo);
    }
}

//update the chess animation manager after each move
void ChessAnimationManager::update(float deltaTime, tModelMap& modelMap) {
    // Process capture animations and remove completed ones
    auto captureIt = captureAnimations.begin();
    while (captureIt != captureAnimations.end()) {
        auto& [pieceId, animation] = *captureIt;
        if (animation.isAnimating) {
            glm::vec3 newPos = animation.getCurrentPosition(deltaTime);
            auto modelIt = modelMap.find(pieceId);
            if (modelIt != modelMap.end()) {
                modelIt->second.tPos = newPos;
                if (animation.isComplete()) {
                    modelMap.erase(modelIt);
                    std::cout << "Removed captured piece: " << pieceId << std::endl;
                    // Remove the completed animation
                    captureIt = captureAnimations.erase(captureIt);
                    continue;
                }
            }
        }
        ++captureIt;
    }

    // Process current move animation
    if (currentAnimation.isAnimating) {
        glm::vec3 newPos = currentAnimation.getCurrentPosition(deltaTime);
        auto it = modelMap.find(currentAnimation.pieceId);
        if (it != modelMap.end()) {
            it->second.tPos = newPos;

            // Handle capture sequence completion
            if (currentAnimation.isComplete() && currentAnimation.waitForCapture && !pendingCapture.empty()) {
                auto [capturedId, capturedPos, isWhite] = pendingCapture.front();
                removePiece(capturedId, capturedPos, isWhite);
                pendingCapture.pop();
            }
        }
    }
    // Start next move if no animations are running
    else if (!pendingMoves.empty()) {
        MoveInfo nextMove = pendingMoves.front();
        pendingMoves.pop();

        if (removedPieces.find(nextMove.pieceId) == removedPieces.end()) {
            if (!nextMove.capturedPieceId.empty()) {
                pendingCapture.push({
                    nextMove.capturedPieceId,
                    nextMove.capturedPiecePos,
                    nextMove.isWhite
                    });

                currentAnimation.startAnimation(
                    nextMove.pieceId,
                    nextMove.startPos,
                    nextMove.endPos,
                    true
                );
            }
            else {
                currentAnimation.startAnimation(
                    nextMove.pieceId,
                    nextMove.startPos,
                    nextMove.endPos,
                    false
                );
            }
        }
    }
}

//special function to handle the removal of a piece
void ChessAnimationManager::removePiece(const std::string& pieceId, const glm::vec3& currentPos, bool isWhite) {
    if (removedPieces.find(pieceId) == removedPieces.end()) {
        //std::cout << "Processing capture for piece: " << pieceId << std::endl;

        float sideOffset = isWhite ? -5.0f : 5.0f;
        int& captureCount = isWhite ? whiteCaptureCount : blackCaptureCount;

        int row = captureCount / 2;
        int col = captureCount % 2;
        float x = sideOffset + (col * CHESS_BOX_SIZE * 0.8f);
        float z = (row - 3.5f) * CHESS_BOX_SIZE;
        glm::vec3 capturePos(x, z, PHEIGHT);

        removedPieces.insert(pieceId);
        captureAnimations[pieceId].startCaptureAnimation(pieceId, currentPos, capturePos);
        captureCount++;
    }
}

//Checks if the animation manager is at work - we dont want to process commands during this time
bool ChessAnimationManager::isAnimating() const {
    if (currentAnimation.isAnimating) return true;
    if (!pendingMoves.empty()) return true;
    if (!pendingCapture.empty()) return true;

    for (const auto& [_, animation] : captureAnimations) {
        if (animation.isAnimating) return true;
    }

    return false;
}

//clear function
void ChessAnimationManager::clear() {
    while (!pendingMoves.empty()) pendingMoves.pop();
    while (!pendingCapture.empty()) pendingCapture.pop();
    currentAnimation.isAnimating = false;
    captureAnimations.clear();
    removedPieces.clear();
    whiteCaptureCount = 0;
    blackCaptureCount = 0;
}