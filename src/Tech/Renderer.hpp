#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>

#include <string>
#include <vector>

#include "Tech/Board.hpp"
#include "Containers/Types.hpp"

class Renderer {
private:
    unsigned int m_shader_program = 0;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;

    static float screenToNdcX(const float &x);
    static float screenToNdcY(const float &y);

    unsigned int compileShader(const unsigned int &type, const char *source);
    unsigned int createShaderProgram();

    void addRectangle(std::vector<Vertex> &vertices,
                      const float &x,
                      const float &y,
                      const float &width,
                      const float &height,
                      const Color &color);

    void addText(std::vector<Vertex> &vertices,
                 const std::string &text,
                 const float &x,
                 const float &y,
                 const float &scale,
                 const Color &color);

    void addCharacter(std::vector<Vertex> &vertices,
                      const char &symbol,
                      const float &x,
                      const float &y,
                      const float &scale,
                      const Color &color);

    void buildBoardVertices(const Board &board, std::vector<Vertex> &vertices);
    void buildInterfaceVertices(std::vector<Vertex> &vertices,
                                const Color &state_color,
                                const float &score_progress,
                                const int &score,
                                const int &score_left,
                                const int &moves_left,
                                const std::string &state_text);

    void drawVertices(const std::vector<Vertex> &vertices);

public:
    bool init();

    void renderGame(const Board &board,
                    const Color &state_color,
                    const float &score_progress,
                    const int &score,
                    const int &score_left,
                    const int &moves_left,
                    const std::string &state_text);

    void shutdown();
};

#endif //RENDERER_H
