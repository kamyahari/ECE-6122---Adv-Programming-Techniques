/*
Author: Kamya Hari
Class: ECE 6122 A
Last date Modified: 12/04/2024

Description:
To create a dynamic 3D chess program. The user inputs their user command move - which is then communicated to the komodo chess engine. The engine replies back with its
best move and the graphics are updated to reflect the moves. The program also detects invalid moves; it also removes the chess piece once it is captured. There are 
other camera controls you can perform from the command prompt.
*/


// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
// User supporting files
#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
// Lab3 specific chess class
#include "chessComponent.h"
#include "chessCommon.h"
#include <thread>
#include <atomic>
#include <windows.h>
#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include "chessAnimation.h" //supporter file for the animation
#include <chrono>

// Sets up the chess board
void setupChessBoard(tModelMap& cTModelMap, const std::vector<chessComponent>& gchessComponents);
void renderChessBoard(
    const std::vector<chessComponent>& components,
    const tModelMap& modelMap,
    const glm::mat4& ProjectionMatrix,
    const glm::mat4& ViewMatrix,
    GLuint MatrixID,
    GLuint ModelMatrixID,
    GLuint ViewMatrixID,
    GLuint LightID,
    GLuint LightPowerID,
    GLuint TextureID,
    const std::set<std::string>& removedPieces);
tModelMap cTModelMap;

//Engine class that initializes and communicates with the komodo engine. 
struct ECE_ChessEngine {
    HANDLE hInputWrite, hInputRead;
    HANDLE hOutputWrite, hOutputRead;
    std::map<std::string, std::pair<glm::vec3, std::string>> boardState; // position -> (3D coords, piece)
    std::string moveHistory;
    ChessAnimationManager animationManager;
    // Add these new members for capture tracking
    int whiteCaptureCount = 0;
    int blackCaptureCount = 0;

    // Add piece type mapping
    struct Piece {
        char type;  // 'P'=pawn, 'R'=rook, 'N'=knight, 'B'=bishop, 'Q'=queen, 'K'=king
        bool isWhite;
    };
    std::map<std::string, Piece> currentBoard;

    void initEngine() { //Initializes the engine
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
        CreatePipe(&hOutputRead, &hOutputWrite, &sa, 0);
        CreatePipe(&hInputRead, &hInputWrite, &sa, 0);

        STARTUPINFO si = { sizeof(STARTUPINFO) };
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hInputRead;
        si.hStdOutput = hOutputWrite;
        si.hStdError = hOutputWrite;

        if (!CreateProcess(NULL, "komodo-14.1-64bit.exe", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
            throw std::runtime_error("Failed to start engine"); //komodo engine initialization
        }

        sendCommand("uci");
        sendCommand("ucinewgame");
        sendCommand("isready");
        initializeBoardState(); //Initialize the Board state
    }

    void ECE_ChessEngine::initializeBoardState() { //Initializes the Board state on startup
        std::map<std::string, std::tuple<std::string, char, bool>> initialSetup = {
            // White pieces 
            {"d1", {"REGINA2_d1", 'Q', true}},
            {"e1", {"RE2_e1", 'K', true}},
            {"a1", {"TORRE3_a1", 'R', true}},
            {"h1", {"TORRE3_h1", 'R', true}},
            {"b1", {"Object3_b1", 'N', true}},
            {"g1", {"Object3_g1", 'N', true}},
            {"c1", {"ALFIERE3_c1", 'B', true}},
            {"f1", {"ALFIERE3_f1", 'B', true}},

            // Black pieces 
            {"d8", {"REGINA01_d8", 'Q', false}},
            {"e8", {"RE01_e8", 'K', false}},
            {"a8", {"TORRE02_a8", 'R', false}},
            {"h8", {"TORRE02_h8", 'R', false}},
            {"b8", {"Object02_b8", 'N', false}},
            {"g8", {"Object02_g8", 'N', false}},
            {"c8", {"ALFIERE02_c8", 'B', false}},
            {"f8", {"ALFIERE02_f8", 'B', false}},

            // White pawns
            {"a2", {"PEDONE13_a2", 'P', true}}, {"b2", {"PEDONE13_b2", 'P', true}},
            {"c2", {"PEDONE13_c2", 'P', true}}, {"d2", {"PEDONE13_d2", 'P', true}},
            {"e2", {"PEDONE13_e2", 'P', true}}, {"f2", {"PEDONE13_f2", 'P', true}},
            {"g2", {"PEDONE13_g2", 'P', true}}, {"h2", {"PEDONE13_h2", 'P', true}},

            // Black pawns
            {"a7", {"PEDONE12_a7", 'P', false}}, {"b7", {"PEDONE12_b7", 'P', false}},
            {"c7", {"PEDONE12_c7", 'P', false}}, {"d7", {"PEDONE12_d7", 'P', false}},
            {"e7", {"PEDONE12_e7", 'P', false}}, {"f7", {"PEDONE12_f7", 'P', false}},
            {"g7", {"PEDONE12_g7", 'P', false}}, {"h7", {"PEDONE12_h7", 'P', false}}
        };

        for (const auto& [pos, pieceInfo] : initialSetup) {
            const auto& [modelId, type, isWhite] = pieceInfo;
            int file = pos[0] - 'a';
            int rank = pos[1] - '1';
            float x = (file - 3.5f) * CHESS_BOX_SIZE;
            float z = (rank - 3.5f) * CHESS_BOX_SIZE;

            currentBoard[pos] = { type, isWhite };
            boardState[pos] = { glm::vec3(x, z, PHEIGHT), modelId };

            /*std::cout << "Initialized " << (isWhite ? "white" : "black") << " "
                << modelId << " at position " << pos
                << " (" << x << ", " << z << ")" << std::endl; print statements for debug (now commented out)*/
        }

        // Print initial board state for verification
        //std::cout << "\nInitial board state:" << std::endl;
        //printBoardState();
    }

    void sendCommand(const std::string& command) { //Sends command to the komodo engine
        DWORD written;
        std::string cmdWithNewline = command + "\n";
        WriteFile(hInputWrite, cmdWithNewline.c_str(), cmdWithNewline.length(), &written, NULL);
        std::cout << "Sent to engine: " << command << std::endl;
    }

    std::string getEngineMove() {
        char buffer[4096];
        DWORD read;
        std::string output;
        std::string bestMove;

        while (true) {
            if (ReadFile(hOutputRead, buffer, sizeof(buffer) - 1, &read, NULL) && read > 0) {
                buffer[read] = '\0';
                output += buffer;

                // Print engine's thinking process
                //std::cout << buffer;

                size_t pos = output.find("bestmove");
                if (pos != std::string::npos) {
                    bestMove = output.substr(pos + 9, 4);
                    std::cout << "\nKomodo plays: " << bestMove << std::endl;
                    return bestMove;
                }
            }
        }
    }

    // Helper function for calculating capture positions
    glm::vec3 calculateCapturePosition(bool isWhite) {
        float sideOffset = isWhite ? -5.0f : 5.0f;  // Left side for white, right side for black
        int& captureCount = isWhite ? whiteCaptureCount : blackCaptureCount;

        // Calculate row and column for captured piece
        int row = captureCount / 2;
        int col = captureCount % 2;

        // Increment counter for next capture
        captureCount++;

        // Position pieces in a grid along the side of the board (Extra credit)
        float x = sideOffset + (col * CHESS_BOX_SIZE * 0.8f);  // Slight scaling for captured pieces
        float z = (row - 3.5f) * CHESS_BOX_SIZE;  // Align with board rows

        return glm::vec3(x, z, PHEIGHT);
    }

    // Add this helper function to check the current state of the board
    /*void printBoardState() {
        std::cout << "\nCurrent Board State:" << std::endl;
        std::cout << "  A B C D E F G H" << std::endl;
        for (int rank = 7; rank >= 0; rank--) {
            std::cout << rank + 1 << " ";
            for (int file = 0; file < 8; file++) {
                std::string pos;
                pos += (char)('a' + file);
                pos += (char)('1' + rank);

                if (boardState.find(pos) != boardState.end()) {
                    std::string pieceId = boardState[pos].second;
                    char pieceChar = '.';

                    // Convert piece ID to display character
                    if (pieceId.find("REGINA") != std::string::npos) pieceChar = 'Q';
                    else if (pieceId.find("RE") != std::string::npos) pieceChar = 'K';
                    else if (pieceId.find("ALFIERE") != std::string::npos) pieceChar = 'B';
                    else if (pieceId.find("Object") != std::string::npos) pieceChar = 'N';
                    else if (pieceId.find("TORRE") != std::string::npos) pieceChar = 'R';
                    else if (pieceId.find("PEDONE") != std::string::npos) pieceChar = 'P';

                    // Make black pieces lowercase
                    if (pieceId.find("01") != std::string::npos ||
                        pieceId.find("02") != std::string::npos ||
                        pieceId.find("12") != std::string::npos) {
                        pieceChar = tolower(pieceChar);
                    }

                    std::cout << pieceChar << " ";
                }
                else {
                    std::cout << ". ";
                }
            }
            std::cout << rank + 1 << std::endl;
        }
        std::cout << "  A B C D E F G H\n" << std::endl;
    }*/

    void ECE_ChessEngine::updateBoardState(const std::string& move) { //update the board state according the moves by the player and the engine
        std::string from = move.substr(0, 2);
        std::string to = move.substr(2, 2);

        if (boardState.find(from) != boardState.end()) {
            // Get moving piece info
            auto movingPiece = boardState[from];
            auto movingPieceBoard = currentBoard[from];

            // Calculate new position
            int toFile = move[2] - 'a';
            int toRank = move[3] - '1';
            float newX = (toFile - 3.5f) * CHESS_BOX_SIZE;
            float newZ = (toRank - 3.5f) * CHESS_BOX_SIZE;
            glm::vec3 newPos(newX, newZ, PHEIGHT);

            // Update board state only, no animation queueing
            boardState.erase(from);
            currentBoard.erase(from);
            boardState[to] = std::make_pair(newPos, movingPiece.second);
            currentBoard[to] = movingPieceBoard;

            std::cout << "Board state updated for piece " << movingPiece.second << std::endl;
        }
        else {
            std::cout << "ERROR: No piece found at position " << from << std::endl;
        }
    }

    bool ECE_ChessEngine::makeMove(const std::string& move) { //actually make the move and render the animation
        if (!isValidMove(move)) {
            std::cout << "Invalid move format. Please use format like 'e2e4'" << std::endl;
            return false;
        }

        std::string from = move.substr(0, 2);
        std::string to = move.substr(2, 2);

        //std::cout << "\n=== Processing Player Move: " << move << " ===" << std::endl;

        // Process player's move - ONLY update board state, don't update position yet
        if (boardState.find(to) != boardState.end()) {
            auto& capturedPiece = boardState[to];
            auto capturedPieceInfo = currentBoard[to];

            // Queue the capture move animation sequence
            auto movingPiece = boardState[from];
            int toFile = to[0] - 'a';
            int toRank = to[1] - '1';
            float newX = (toFile - 3.5f) * CHESS_BOX_SIZE;
            float newZ = (toRank - 3.5f) * CHESS_BOX_SIZE;

            animationManager.queueCaptureMove(
                movingPiece.second,
                movingPiece.first,
                glm::vec3(newX, newZ, PHEIGHT),
                capturedPiece.second,
                capturedPiece.first,
                capturedPieceInfo.isWhite
            );

            // Update board state tracking
            boardState.erase(to);
            currentBoard.erase(to);
        }
        else {
            // Normal move without capture
            auto movingPiece = boardState[from];
            int toFile = to[0] - 'a';
            int toRank = to[1] - '1';
            float newX = (toFile - 3.5f) * CHESS_BOX_SIZE;
            float newZ = (toRank - 3.5f) * CHESS_BOX_SIZE;

            animationManager.queueMove(
                movingPiece.second,
                movingPiece.first,
                glm::vec3(newX, newZ, PHEIGHT)
            );
        }

        // Update board state after queueing animation
        updateBoardState(move);  // This updates the internal board representation

        // Send move to engine and get response
        std::string command = "position startpos moves " + moveHistory + " " + move;
        sendCommand(command);
        sendCommand("go depth 10");

        // Get and validate engine's response
        std::string engineMove = getEngineMove();

        // Process engine's move
        if (engineMove.length() == 4) {
            std::string engineFrom = engineMove.substr(0, 2);
            std::string engineTo = engineMove.substr(2, 2);

            // Queue engine's move animation
            if (boardState.find(engineTo) != boardState.end()) {
                auto& capturedPiece = boardState[engineTo];
                auto capturedPieceInfo = currentBoard[engineTo];
                auto enginePiece = boardState[engineFrom];

                int toFile = engineTo[0] - 'a';
                int toRank = engineTo[1] - '1';
                float newX = (toFile - 3.5f) * CHESS_BOX_SIZE;
                float newZ = (toRank - 3.5f) * CHESS_BOX_SIZE;

                animationManager.queueCaptureMove(
                    enginePiece.second,
                    enginePiece.first,
                    glm::vec3(newX, newZ, PHEIGHT),
                    capturedPiece.second,
                    capturedPiece.first,
                    capturedPieceInfo.isWhite
                );

                boardState.erase(engineTo);
                currentBoard.erase(engineTo);
            }
            else {
                auto enginePiece = boardState[engineFrom];
                int toFile = engineTo[0] - 'a';
                int toRank = engineTo[1] - '1';
                float newX = (toFile - 3.5f) * CHESS_BOX_SIZE;
                float newZ = (toRank - 3.5f) * CHESS_BOX_SIZE;

                animationManager.queueMove(
                    enginePiece.second,
                    enginePiece.first,
                    glm::vec3(newX, newZ, PHEIGHT)
                );
            }

            // Update board state for engine's move
            updateBoardState(engineMove);
            moveHistory += " " + move + " " + engineMove;
        }

        return true;
    }

    void ECE_ChessEngine::updatePosition(const std::string& move) { // Actual position updation
        std::string from = move.substr(0, 2);
        std::string to = move.substr(2, 2);

        if (boardState.find(from) != boardState.end()) {
            // Get moving piece info
            auto movingPiece = boardState[from];
            auto movingPieceBoard = currentBoard[from];
            std::string movingPieceId = movingPiece.second;

            // Calculate new position
            int toFile = move[2] - 'a';
            int toRank = move[3] - '1';
            float newX = (toFile - 3.5f) * CHESS_BOX_SIZE;
            float newZ = (toRank - 3.5f) * CHESS_BOX_SIZE;
            glm::vec3 newPos(newX, newZ, PHEIGHT);

            /*std::cout << "Updating position for piece " << movingPieceId << ":" << std::endl;
            std::cout << "  From: " << from << " (" << movingPiece.first.x << ", "
                << movingPiece.first.y << ", " << movingPiece.first.z << ")" << std::endl;
            std::cout << "  To: " << to << " (" << newX << ", " << newZ << ", "
                << PHEIGHT << ")" << std::endl;*/

            // Update board state
            boardState.erase(from);
            currentBoard.erase(from);
            boardState[to] = std::make_pair(newPos, movingPieceId);
            currentBoard[to] = movingPieceBoard;

            // Queue the move animation
            animationManager.queueMove(movingPieceId, movingPiece.first, newPos);

            //std::cout << "Position updated successfully" << std::endl;
        }
        else {
            std::cout << "ERROR: No piece found at position " << from << std::endl;
        }
    }

    bool isValidMove(const std::string& move) { //check if the user move/komodo's move is valid or not
        if (move.length() != 4) {
            return false;
        }

        std::string from = move.substr(0, 2);
        std::string to = move.substr(2, 2);

        // Basic format check
        if (!isalpha(move[0]) || !isdigit(move[1]) ||
            !isalpha(move[2]) || !isdigit(move[3])) {
            std::cout << "Invalid move format" << std::endl;
            return false;
        }

        // Check if source square has a piece
        if (boardState.find(from) == boardState.end()) {
            std::cout << "No piece at " << from << std::endl;
            return false;
        }

        // Get piece information
        auto& piece = currentBoard[from];
        bool isPlayerPiece = piece.isWhite;  // true for white pieces

        // Make sure player is moving their own piece
        if (!isPlayerPiece) {
            std::cout << "Cannot move opponent's piece" << std::endl;
            return false;
        }

        bool isWhitePiece = piece.isWhite;
        bool isWhiteTurn = !(moveHistory.length() / 5 % 2); // Even number of moves means White's turn

        // Check if correct player is moving
        if (isWhitePiece != isWhiteTurn) {
            std::cout << (isWhiteTurn ? "White" : "Black") << "'s turn to move" << std::endl;
            return false;
        }

        // Get move coordinates
        int fromFile = from[0] - 'a';
        int fromRank = from[1] - '1';
        int toFile = to[0] - 'a';
        int toRank = to[1] - '1';

        // Check if destination has friendly piece
        if (boardState.find(to) != boardState.end()) {
            if (currentBoard[to].isWhite == isPlayerPiece) {
                std::cout << "Cannot capture own piece" << std::endl;
                return false;
            }
        }

        // Validate piece-specific moves
        switch (piece.type) {
        case 'P': // Pawn
            if (!isValidPawnMove(fromFile, fromRank, toFile, toRank, isPlayerPiece, to)) {
                std::cout << "Invalid pawn move" << std::endl;
                return false;
            }
            break;

        case 'N': // Knight
            if (!isValidKnightMove(fromFile, fromRank, toFile, toRank)) {
                std::cout << "Invalid knight move" << std::endl;
                return false;
            }
            break;

        case 'B': // Bishop
            if (!isValidDiagonalMove(fromFile, fromRank, toFile, toRank)) {
                std::cout << "Invalid bishop move" << std::endl;
                return false;
            }
            break;

        case 'R': // Rook
            if (!isValidStraightMove(fromFile, fromRank, toFile, toRank)) {
                std::cout << "Invalid rook move" << std::endl;
                return false;
            }
            break;

        case 'Q': // Queen
            if (!isValidStraightMove(fromFile, fromRank, toFile, toRank) &&
                !isValidDiagonalMove(fromFile, fromRank, toFile, toRank)) {
                std::cout << "Invalid queen move" << std::endl;
                return false;
            }
            break;

        case 'K': // King
            if (!isValidKingMove(fromFile, fromRank, toFile, toRank)) {
                std::cout << "Invalid king move" << std::endl;
                return false;
            }
            break;
        }

        return true;
    }

    bool isValidPawnMove(int fromFile, int fromRank, int toFile, int toRank, bool isWhite, const std::string& to) { //check if it is a valid pawn move
        int direction = isWhite ? 1 : -1;
        int startRank = isWhite ? 1 : 6;

        // Forward move
        if (fromFile == toFile) {
            // Single step
            if (toRank - fromRank == direction) {
                return boardState.find(to) == boardState.end(); // Must be empty
            }
            // Double step from starting position
            if (fromRank == startRank && toRank - fromRank == 2 * direction) {
                // Check if path is clear
                std::string midSquare;
                midSquare += (char)('a' + fromFile);
                midSquare += (char)('1' + fromRank + direction);
                return boardState.find(to) == boardState.end() &&
                    boardState.find(midSquare) == boardState.end();
            }
        }

        // Capture move
        if (abs(toFile - fromFile) == 1 && toRank - fromRank == direction) {
            return boardState.find(to) != boardState.end(); // Must have enemy piece
        }

        return false;
    }

    bool isValidKnightMove(int fromFile, int fromRank, int toFile, int toRank) { //checks if it is a valid knight move
        int dx = abs(toFile - fromFile);
        int dy = abs(toRank - fromRank);
        return (dx == 2 && dy == 1) || (dx == 1 && dy == 2);
    }

    bool isValidDiagonalMove(int fromFile, int fromRank, int toFile, int toRank) { //checks if it is a valid diagonal move
        // Check if move is diagonal
        if (abs(toFile - fromFile) != abs(toRank - fromRank)) {
            std::cout << "Not a diagonal move" << std::endl;
            return false;
        }

        // Determine direction
        int fileStep = (toFile > fromFile) ? 1 : -1;
        int rankStep = (toRank > fromRank) ? 1 : -1;

        // Check path for obstacles
        int currentFile = fromFile + fileStep;
        int currentRank = fromRank + rankStep;

        while (currentFile != toFile) {
            std::string square;
            square += (char)('a' + currentFile);
            square += (char)('1' + currentRank);

            if (boardState.find(square) != boardState.end()) {
                std::cout << "Path blocked at " << square << std::endl;
                return false;  // Path is blocked
            }

            currentFile += fileStep;
            currentRank += rankStep;
        }

        return true;
    }

    bool isValidStraightMove(int fromFile, int fromRank, int toFile, int toRank) { //check if it is a valid straight path
        if (fromFile != toFile && fromRank != toRank) {
            return false;
        }

        // Check if path is clear
        if (fromFile == toFile) {
            int step = (toRank - fromRank) > 0 ? 1 : -1;
            for (int rank = fromRank + step; rank != toRank; rank += step) {
                std::string square;
                square += (char)('a' + fromFile);
                square += (char)('1' + rank);

                if (boardState.find(square) != boardState.end()) {
                    return false; // Path is blocked
                }
            }
        }
        else {
            int step = (toFile - fromFile) > 0 ? 1 : -1;
            for (int file = fromFile + step; file != toFile; file += step) {
                std::string square;
                square += (char)('a' + file);
                square += (char)('1' + fromRank);

                if (boardState.find(square) != boardState.end()) {
                    return false; // Path is blocked
                }
            }
        }

        return true;
    }

    bool isValidKingMove(int fromFile, int fromRank, int toFile, int toRank) { //check if the move is valid for King
        return abs(toFile - fromFile) <= 1 && abs(toRank - fromRank) <= 1;
    }

    bool ECE_ChessEngine::isSquareUnderAttack(const std::string& square, bool byWhite) { //Check for Capture
        // Check all opponent pieces to see if they can attack this square
        for (const auto& [pos, piece] : currentBoard) {
            if (piece.isWhite == byWhite) {  // Check only pieces of attacking color
                // Get coordinates
                int fromFile = pos[0] - 'a';
                int fromRank = pos[1] - '1';
                int toFile = square[0] - 'a';
                int toRank = square[1] - '1';

                bool canAttack = false;
                switch (piece.type) {
                case 'P':
                    // Pawns attack diagonally
                    if (byWhite) {
                        canAttack = (abs(toFile - fromFile) == 1 && (toRank - fromRank) == 1);
                    }
                    else {
                        canAttack = (abs(toFile - fromFile) == 1 && (fromRank - toRank) == 1);
                    }
                    break;
                case 'N':
                    canAttack = isValidKnightMove(fromFile, fromRank, toFile, toRank);
                    break;
                case 'B':
                    canAttack = isValidDiagonalMove(fromFile, fromRank, toFile, toRank);
                    break;
                case 'R':
                    canAttack = isValidStraightMove(fromFile, fromRank, toFile, toRank);
                    break;
                case 'Q':
                    canAttack = isValidStraightMove(fromFile, fromRank, toFile, toRank) ||
                        isValidDiagonalMove(fromFile, fromRank, toFile, toRank);
                    break;
                case 'K':
                    canAttack = isValidKingMove(fromFile, fromRank, toFile, toRank);
                    break;
                }
                if (canAttack) return true;
            }
        }
        return false;
    }

    bool ECE_ChessEngine::isInCheck(bool whiteKing) { //Check condition
        // Find the king's position
        std::string kingPos;
        for (const auto& [pos, piece] : currentBoard) {
            if (piece.type == 'K' && piece.isWhite == whiteKing) {
                kingPos = pos;
                break;
            }
        }

        // Check if the king's square is under attack by opponent pieces
        return isSquareUnderAttack(kingPos, !whiteKing);
    }

    bool ECE_ChessEngine::canBlockCheck(const std::string& kingPos, bool isWhiteKing) { //check if it is just check or a checkmate
        // Try all pieces of the same color to see if they can make a legal move
        for (const auto& [fromPos, piece] : currentBoard) {
            if (piece.isWhite == isWhiteKing) {
                // Try all possible destination squares
                for (int toRank = 0; toRank < 8; toRank++) {
                    for (int toFile = 0; toFile < 8; toFile++) {
                        std::string toPos;
                        toPos += (char)('a' + toFile);
                        toPos += (char)('1' + toRank);

                        // Create the move string
                        std::string testMove = fromPos + toPos;

                        // Check if this would be a valid move
                        if (isValidMove(testMove)) {
                            // Make the move temporarily
                            auto tempBoard = currentBoard;
                            auto tempState = boardState;

                            // Simulate the move
                            updatePosition(testMove);

                            // Check if this gets us out of check
                            if (!isInCheck(isWhiteKing)) {
                                // Restore board state
                                currentBoard = tempBoard;
                                boardState = tempState;
                                return true;
                            }

                            // Restore board state
                            currentBoard = tempBoard;
                            boardState = tempState;
                        }
                    }
                }
            }
        }
        return false;
    }

    bool ECE_ChessEngine::isCheckmate(bool whiteKing) { //Checks if the game has a checkmate
        // First, check if the king is in check
        if (!isInCheck(whiteKing)) {
            return false;
        }

        // Find the king's position
        std::string kingPos;
        for (const auto& [pos, piece] : currentBoard) {
            if (piece.type == 'K' && piece.isWhite == whiteKing) {
                kingPos = pos;
                break;
            }
        }

        // If we can block the check, it's not checkmate
        if (canBlockCheck(kingPos, whiteKing)) {
            return false;
        }

        // Try all possible king moves to see if the king can escape
        int kingFile = kingPos[0] - 'a';
        int kingRank = kingPos[1] - '1';

        for (int rankOffset = -1; rankOffset <= 1; rankOffset++) {
            for (int fileOffset = -1; fileOffset <= 1; fileOffset++) {
                if (fileOffset == 0 && rankOffset == 0) continue;

                int newFile = kingFile + fileOffset;
                int newRank = kingRank + rankOffset;

                // Check if the new position is on the board
                if (newFile >= 0 && newFile < 8 && newRank >= 0 && newRank < 8) {
                    std::string newPos;
                    newPos += (char)('a' + newFile);
                    newPos += (char)('1' + newRank);

                    // Check if the king can move to this square
                    std::string testMove = kingPos + newPos;
                    if (isValidMove(testMove)) {
                        // Make the move temporarily
                        auto tempBoard = currentBoard;
                        auto tempState = boardState;

                        updatePosition(testMove);

                        // Check if this square is safe
                        if (!isSquareUnderAttack(newPos, !whiteKing)) {
                            // Restore board state
                            currentBoard = tempBoard;
                            boardState = tempState;
                            return false;
                        }

                        // Restore board state
                        currentBoard = tempBoard;
                        boardState = tempState;
                    }
                }
            }
        }

        // If we get here, no escape was found
        return true;
    }
};

// MAIN FUNCTION TO RENDER THE LOOP

int main(void)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }
    ECE_ChessEngine chess; //Initialize engine
    try {
        chess.initEngine();
    }
    catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Game Of Chess 3D", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited movement
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    //glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Get a handle for our "lightToggleSwitch" uniform
    GLuint LightSwitchID = glGetUniformLocation(programID, "lightSwitch");

    // Create a vector of chess components class
    // Each component is fully self sufficient
    std::vector<chessComponent> gchessComponents;

    // Load the OBJ files
    bool cBoard = loadAssImpLab3("Lab3/Stone_Chess_Board/12951_Stone_Chess_Board_v1_L3.obj", gchessComponents); //load the components
    bool cComps = loadAssImpLab3("Lab3/Chess/chess-mod.obj", gchessComponents);

    // Proceed iff OBJ loading is successful
    if (!cBoard || !cComps)
    {
        // Quit the program (Failed OBJ loading)
        std::cout << "Program failed due to OBJ loading failure, please CHECK!" << std::endl;
        return -1;
    }
    std::cout << "\nLoaded " << gchessComponents.size() << " components before setup\n";
    // Setup the Chess board locations
    tModelMap cTModelMap;
    setupChessBoard(cTModelMap, gchessComponents); //setuo the chess board

    /*computeMatricesFromInputsLab3(); // Initialize camera matrices before first render
    glm::mat4 ProjectionMatrix = getProjectionMatrix();
    glm::mat4 ViewMatrix = getViewMatrix();*/

    // Load it into a VBO (One time activity)
    // Run through all the components for rendering
    for (auto cit = gchessComponents.begin(); cit != gchessComponents.end(); cit++)
    {
        // Setup VBO buffers
        cit->setupGLBuffers();
        // Setup Texture
        cit->setupTextureBuffers();
    }

    // Use our shader (Not changing the shader per chess component)
    glUseProgram(programID);

    // Get a handle for our "LightPosition" uniform
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // Get light power uniform location
    GLuint LightPowerID = glGetUniformLocation(programID, "LightPower");

    // Reset mouse position for stability
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);
    // Force initial computation of matrices
    computeMatricesFromInputsLab3();

    // Time tracking for animation
    double lastTime = glfwGetTime();
    double currentTime;
    float deltaTime;
    bool isProcessingMove = false; //to check if a move is being processed

    do {
        // Compute frame timing
        currentTime = glfwGetTime();
        deltaTime = float(currentTime - lastTime);
        lastTime = currentTime;

        // Update animations
        chess.animationManager.update(deltaTime, cTModelMap);

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute the VP matrix from keyboard input
        computeMatricesFromInputsLab3();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        // Get light switch State
        bool lightSwitch = getLightSwitch();
        glUniform1i(LightSwitchID, static_cast<int>(lightSwitch));

        // Render the chess board with current state
        renderChessBoard(
            gchessComponents,
            cTModelMap,
            ProjectionMatrix,
            ViewMatrix,
            MatrixID,
            ModelMatrixID,
            ViewMatrixID,
            LightID,
            LightPowerID,
            TextureID,
            chess.animationManager.removedPieces
        );

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        // Only process input when no animation is running and not already processing a move
        if (!chess.animationManager.isAnimating() && !isProcessingMove) {
            
                isProcessingMove = true;  // Set flag before processing
                std::string input;
                std::cout << "\nEnter command (type 'help' for options): ";
                std::getline(std::cin, input);

                std::stringstream ss(input);
                std::string cmd;
                ss >> cmd;

                if (cmd == "quit") {
                    std::cout << "Thank you for playing!!" << std::endl;
                    break;
                }
                else if (cmd == "move") { //process move
                    std::string moveStr;
                    ss >> moveStr;
                    if (!moveStr.empty()) {
                        if (!chess.makeMove(moveStr)) {
                            std::cout << "Invalid move. Please use format: move e2e4\n";
                        }
                    }
                }
                else if (cmd == "help") {
                    std::cout << "\nAvailable commands:\n"
                        << "  move e2e4  - Make a chess move (use algebraic notation)\n"
                        << "  quit       - Exit the program\n"
                        << "  help       - Show this help message\n";
                }
                else if (!input.empty()) {
                    processCommand(input);
                }
                isProcessingMove = false;  // Reset flag after processing
        }

        // Add a small sleep to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);
    // Cleanup
    //shouldExit = true;
    //cmdThread.join();

    // Cleanup VBO, Texture (Done in class destructor) and shader 
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    // Shake hand for exit!
    return 0;
}


void setupChessBoard(tModelMap& cTModelMap, const std::vector<chessComponent>& gchessComponents) { //setup the chess board
    cTModelMap.clear();

    // Chess board setup
    cTModelMap["12951_Stone_Chess_Board"] = {
        1, 0, 0.f, {1, 0, 0},
        glm::vec3(CBSCALE),
        {0.f, 0.f, PHEIGHT}
    };

    // Define piece positions with unique IDs
    struct PieceSetup {
        std::string baseId;   // Original piece ID from OBJ
        std::string uniqueId; // Unique ID for this instance
        std::string pos;      // Chess position
        bool isWhite;
    };

    std::vector<PieceSetup> pieceSetups = {
        // White Rooks
        {"TORRE3", "TORRE3_a1", "a1", true}, {"TORRE3", "TORRE3_h1", "h1", true},
        // White Knights
        {"Object3", "Object3_b1", "b1", true}, {"Object3", "Object3_g1", "g1", true},
        // White Bishops
        {"ALFIERE3", "ALFIERE3_c1", "c1", true}, {"ALFIERE3", "ALFIERE3_f1", "f1", true},
        // White Queen & King
        {"REGINA2", "REGINA2_d1", "d1", true}, {"RE2", "RE2_e1", "e1", true},

        // Black Rooks
        {"TORRE02", "TORRE02_a8", "a8", false}, {"TORRE02", "TORRE02_h8", "h8", false},
        // Black Knights
        {"Object02", "Object02_b8", "b8", false}, {"Object02", "Object02_g8", "g8", false},
        // Black Bishops
        {"ALFIERE02", "ALFIERE02_c8", "c8", false}, {"ALFIERE02", "ALFIERE02_f8", "f8", false},
        // Black Queen & King
        {"REGINA01", "REGINA01_d8", "d8", false}, {"RE01", "RE01_e8", "e8", false},
    };

    // Add white pawns
    for (char file = 'a'; file <= 'h'; file++) {
        pieceSetups.push_back({
            "PEDONE13",
            "PEDONE13_" + std::string(1, file) + "2",
            std::string(1, file) + "2",
            true
            });
    }

    // Add black pawns
    for (char file = 'a'; file <= 'h'; file++) {
        pieceSetups.push_back({
            "PEDONE12",
            "PEDONE12_" + std::string(1, file) + "7",
            std::string(1, file) + "7",
            false
            });
    }

    // Set up all pieces
    for (const auto& setup : pieceSetups) {
        int file = setup.pos[0] - 'a';
        int rank = setup.pos[1] - '1';
        float x = (file - 3.5f) * CHESS_BOX_SIZE;
        float z = (rank - 3.5f) * CHESS_BOX_SIZE;

        cTModelMap[setup.uniqueId] = {
            1,      // count
            0,      // distance
            90.f,   // rotation angle
            {1, 0, 0},  // rotation axis
            glm::vec3(CPSCALE),  // scale
            {x, z, PHEIGHT}  // position
        };

        /*std::cout << "Set up " << (setup.isWhite ? "white" : "black")
            << " piece " << setup.baseId
            << " with ID " << setup.uniqueId
            << " at position " << setup.pos
            << " (" << x << ", " << z << ", " << PHEIGHT << ")\n";*/
    }

    //std::cout << "\nBoard setup complete. Total pieces in map: " << cTModelMap.size() << std::endl;
}


void renderChessBoard(
    const std::vector<chessComponent>& components,
    const tModelMap& modelMap,
    const glm::mat4& ProjectionMatrix,
    const glm::mat4& ViewMatrix,
    GLuint MatrixID,
    GLuint ModelMatrixID,
    GLuint ViewMatrixID,
    GLuint LightID,
    GLuint LightPowerID,
    GLuint TextureID,
    const std::set<std::string>& removedPieces) { //Render the whole chess board - as the game progresses

    // First render the board
    for (const auto& component : components) {
        if (component.getComponentID() == "12951_Stone_Chess_Board") {
            auto it = modelMap.find("12951_Stone_Chess_Board");
            if (it != modelMap.end()) {
                const tPosition& pos = it->second;
                glm::mat4 ModelMatrix = component.genModelMatrix(pos);
                glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
                glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

                glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
                glUniform1f(LightPowerID, lightPower);

                component.setupTexture(TextureID);
                component.renderMesh();
            }
            break;  // Board rendered, move on to pieces
        }
    }

    // Then render all pieces that haven't been captured
    for (const auto& component : components) {
        std::string baseId = component.getComponentID();
        if (baseId == "12951_Stone_Chess_Board") continue;

        // Find all pieces that use this base mesh
        for (const auto& [uniqueId, position] : modelMap) {
            // Skip if this piece has been captured
            if (removedPieces.find(uniqueId) != removedPieces.end()) {
                continue;
            }

            // Check if this unique ID starts with the base ID
            if (uniqueId.find(baseId) == 0) {
                glm::mat4 ModelMatrix = component.genModelMatrix(position);
                glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
                glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

                glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
                glUniform1f(LightPowerID, lightPower);

                component.setupTexture(TextureID);
                component.renderMesh();
            }
        }
    }
}