#include "Renderer.hpp"

#include <array>
#include <cctype>
#include <cstddef>
#include <string>

#include "Containers/Config.hpp"

namespace {
const char *vertex_shader_source = R"(
#version 330 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vertexColor = aColor;
}
)";

const char *fragment_shader_source = R"(
#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";

const std::array<Color, COLOR_COUNT> CELL_COLORS = {
        Color{0.90f, 0.20f, 0.25f},
        Color{0.20f, 0.65f, 0.95f},
        Color{0.25f, 0.80f, 0.35f},
        Color{0.95f, 0.80f, 0.20f},
        Color{0.70f, 0.35f, 0.95f},
        Color{0.95f, 0.50f, 0.20f}
};

float clamp01(const float &value) {
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

std::array<unsigned char, 7> getPattern(const char &symbol) {
    switch (symbol) {
        case '0': return {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110};
        case '1': return {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
        case '2': return {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111};
        case '3': return {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110};
        case '4': return {0b10010, 0b10010, 0b10010, 0b11111, 0b00010, 0b00010, 0b00010};
        case '5': return {0b11111, 0b10000, 0b10000, 0b11110, 0b00001, 0b00001, 0b11110};
        case '6': return {0b01110, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110};
        case '7': return {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000};
        case '8': return {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110};
        case '9': return {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b01110};

        case 'A': return {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
        case 'B': return {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110};
        case 'C': return {0b01110, 0b10001, 0b10000, 0b10000, 0b10000, 0b10001, 0b01110};
        case 'D': return {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110};
        case 'E': return {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111};
        case 'F': return {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000};
        case 'G': return {0b01110, 0b10001, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111};
        case 'H': return {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001};
        case 'I': return {0b01110, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110};
        case 'J': return {0b00111, 0b00010, 0b00010, 0b00010, 0b10010, 0b10010, 0b01100};
        case 'K': return {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001};
        case 'L': return {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111};
        case 'M': return {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001};
        case 'N': return {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001};
        case 'O': return {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
        case 'P': return {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000};
        case 'Q': return {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101};
        case 'R': return {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001};
        case 'S': return {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110};
        case 'T': return {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100};
        case 'U': return {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110};
        case 'V': return {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100};
        case 'W': return {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b10101, 0b01010};
        case 'X': return {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001};
        case 'Y': return {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100};
        case 'Z': return {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111};

        case ':': return {0b00000, 0b00100, 0b00100, 0b00000, 0b00100, 0b00100, 0b00000};
        case '/': return {0b00001, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b10000};
        case '-': return {0b00000, 0b00000, 0b00000, 0b11111, 0b00000, 0b00000, 0b00000};
        case '!': return {0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00000, 0b00100};
        default: return {0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000};
    }
}
}

bool Renderer::init() {
    this->m_shader_program = createShaderProgram();

    if (this->m_shader_program == 0) {
        return false;
    }

    glGenVertexArrays(1, &this->m_vao);
    glGenBuffers(1, &this->m_vbo);

    glBindVertexArray(this->m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->m_vbo);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, x)));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, r)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void Renderer::shutdown() {
    if (this->m_vbo != 0) {
        glDeleteBuffers(1, &this->m_vbo);
        this->m_vbo = 0;
    }
    if (this->m_vao != 0) {
        glDeleteVertexArrays(1, &this->m_vao);
        this->m_vao = 0;
    }
    if (this->m_shader_program != 0) {
        glDeleteProgram(this->m_shader_program);
        this->m_shader_program = 0;
    }
}

float Renderer::screenToNdcX(const float &x) {
    return (x / WINDOW_WIDTH) * 2.0f - 1.0f;
}

float Renderer::screenToNdcY(const float &y) {
    return 1.0f - (y / WINDOW_HEIGHT) * 2.0f;
}

unsigned int Renderer::compileShader(const unsigned int &type, const char *source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

unsigned int Renderer::createShaderProgram() {
    unsigned int vertex_shader = compileShader(GL_VERTEX_SHADER, vertex_shader_source);
    if (vertex_shader == 0) {
        return 0;
    }

    unsigned int fragment_shader = compileShader(GL_FRAGMENT_SHADER, fragment_shader_source);
    if (fragment_shader == 0) {
        glDeleteShader(vertex_shader);
        return 0;
    }

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    int success = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return program;
}

void Renderer::addRectangle(std::vector<Vertex> &vertices,
                            const float &x,
                            const float &y,
                            const float &width,
                            const float &height,
                            const Color &color) {
    float left = screenToNdcX(x);
    float right = screenToNdcX(x + width);
    float top = screenToNdcY(y);
    float bottom = screenToNdcY(y + height);

    vertices.push_back({left, top, color.r, color.g, color.b});
    vertices.push_back({right, top, color.r, color.g, color.b});
    vertices.push_back({right, bottom, color.r, color.g, color.b});

    vertices.push_back({left, top, color.r, color.g, color.b});
    vertices.push_back({right, bottom, color.r, color.g, color.b});
    vertices.push_back({left, bottom, color.r, color.g, color.b});
}

void Renderer::addCharacter(std::vector<Vertex> &vertices,
                            const char &symbol,
                            const float &x,
                            const float &y,
                            const float &scale,
                            const Color &color) {
    char upper_symbol = static_cast<char>(std::toupper(static_cast<unsigned char>(symbol)));
    std::array<unsigned char, 7> pattern = getPattern(upper_symbol);

    for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
            unsigned char mask = static_cast<unsigned char>(1 << (4 - col));

            if ((pattern[row] & mask) == 0) {
                continue;
            }

            addRectangle(vertices,
                         x + static_cast<float>(col) * scale,
                         y + static_cast<float>(row) * scale,
                         scale,
                         scale,
                         color);
        }
    }
}

void Renderer::addText(std::vector<Vertex> &vertices,
                       const std::string &text,
                       const float &x,
                       const float &y,
                       const float &scale,
                       const Color &color) {
    float cursor_x = x;

    for (char symbol: text) {
        if (symbol == ' ') {
            cursor_x += scale * 4.0f;
            continue;
        }

        addCharacter(vertices, symbol, cursor_x, y, scale, color);
        cursor_x += scale * 6.0f;
    }
}

void Renderer::buildBoardVertices(const Board &board, std::vector<Vertex> &vertices) {
    const float gap = 4.0f;

    for (int row = 0; row < BOARD_ROWS; row++) {
        for (int col = 0; col < BOARD_COLS; col++) {
            const Cell &cell = board.getCell(row, col);

            if (cell.color_index == EMPTY_CELL) {
                continue;
            }

            float x = BOARD_OFFSET_X + col * CELL_SIZE;
            float y = BOARD_OFFSET_Y + row * CELL_SIZE;
            Color cell_color = CELL_COLORS[cell.color_index];

            addRectangle(vertices,
                         x + gap,
                         y + gap,
                         CELL_SIZE - gap * 2.0f,
                         CELL_SIZE - gap * 2.0f,
                         cell_color);

            if (cell.selected) {
                addRectangle(vertices,
                             x + 12.0f,
                             y + 12.0f,
                             CELL_SIZE - 24.0f,
                             CELL_SIZE - 24.0f,
                             Color{1.0f, 1.0f, 1.0f});
            }
        }
    }
}

void Renderer::buildInterfaceVertices(std::vector<Vertex> &vertices,
                                      const Color &state_color,
                                      const float &score_progress,
                                      const int &score,
                                      const int &score_left,
                                      const int &moves_left,
                                      const std::string &state_text) {
    float score_progress_clamped = clamp01(score_progress);

    const float frame_padding = 10.0f;
    const float frame_thickness = 8.0f;

    float frame_x = BOARD_OFFSET_X - frame_padding;
    float frame_y = BOARD_OFFSET_Y - frame_padding;
    float frame_width = BOARD_PIXEL_WIDTH + frame_padding * 2.0f;
    float frame_height = BOARD_PIXEL_HEIGHT + frame_padding * 2.0f;

    addRectangle(vertices, frame_x, frame_y, frame_width, frame_thickness, state_color);
    addRectangle(vertices, frame_x, frame_y + frame_height - frame_thickness, frame_width, frame_thickness, state_color);
    addRectangle(vertices, frame_x, frame_y, frame_thickness, frame_height, state_color);
    addRectangle(vertices, frame_x + frame_width - frame_thickness, frame_y, frame_thickness, frame_height, state_color);

    const float bar_height = 18.0f;
    const float bar_y_score = 52.0f;

    Color text_color{0.92f, 0.94f, 1.0f};
    Color bar_background{0.18f, 0.19f, 0.23f};

    addText(vertices, "SCORE:" + std::to_string(score), 35.0f, 18.0f, 3.0f, text_color);
    addText(vertices, "NEED:" + std::to_string(score_left), 310.0f, 18.0f, 3.0f, text_color);
    addText(vertices, "MOVES:" + std::to_string(moves_left), 565.0f, 18.0f, 3.0f, text_color);

    if (state_text == "YOU WON") {
        addText(vertices, "YOU WON! PRESS R TO RESTART", 157.0f, WINDOW_HEIGHT - 34.0f, 3.0f, state_color);
    }
    if (state_text == "YOU LOST") {
        addText(vertices, "PRESS R TO RESTART", 255.0f, WINDOW_HEIGHT - 34.0f, 3.0f, state_color);
    }

    addRectangle(vertices, BOARD_OFFSET_X, bar_y_score, BOARD_PIXEL_WIDTH, bar_height, bar_background);
    addRectangle(vertices, BOARD_OFFSET_X, bar_y_score, BOARD_PIXEL_WIDTH * score_progress_clamped, bar_height, state_color);
}

void Renderer::drawVertices(const std::vector<Vertex> &vertices) {
    glUseProgram(this->m_shader_program);

    glBindVertexArray(this->m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->m_vbo);

    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                 vertices.data(),
                 GL_DYNAMIC_DRAW);

    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::renderGame(const Board &board,
                          const Color &state_color,
                          const float &score_progress,
                          const int &score,
                          const int &score_left,
                          const int &moves_left,
                          const std::string &state_text) {
    std::vector<Vertex> vertices;

    buildBoardVertices(board, vertices);
    buildInterfaceVertices(vertices,
                           state_color,
                           score_progress,
                           score,
                           score_left,
                           moves_left,
                           state_text);
    drawVertices(vertices);
}
