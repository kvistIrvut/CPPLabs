#include "Renderer.hpp"

#include <array>
#include <cmath>
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

float clamp01(const float &value) {
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

Color getBrickColor(const Brick &brick) {
    switch (brick.type) {
        case BrickType::Normal:
            if (brick.health >= 3) {
                return {0.85f, 0.25f, 0.30f};
            }
            if (brick.health == 2) {
                return {0.95f, 0.62f, 0.22f};
            }
            return {0.25f, 0.60f, 0.95f};
        case BrickType::Unbreakable:
            return {0.46f, 0.48f, 0.54f};
        case BrickType::SpeedUp:
            return {0.95f, 0.36f, 0.15f};
        default:
            return {0.0f, 0.0f, 0.0f};
    }
}

Color getBonusColor(const BonusType &type) {
    switch (type) {
        case BonusType::PaddleGrow:
            return {0.20f, 0.80f, 0.35f};
        case BonusType::PaddleShrink:
            return {0.95f, 0.30f, 0.25f};
        case BonusType::BallSpeedUp:
            return {0.95f, 0.65f, 0.20f};
        case BonusType::BallSpeedDown:
            return {0.25f, 0.65f, 0.95f};
        case BonusType::StickyPaddle:
            return {0.70f, 0.45f, 0.95f};
        case BonusType::SafetyFloor:
            return {0.20f, 0.85f, 0.78f};
        case BonusType::RandomDirection:
            return {0.92f, 0.92f, 0.30f};
        default:
            return {1.0f, 1.0f, 1.0f};
    }
}

std::string getBonusText(const BonusType &type) {
    switch (type) {
        case BonusType::PaddleGrow:
            return "W";
        case BonusType::PaddleShrink:
            return "N";
        case BonusType::BallSpeedUp:
            return "F";
        case BonusType::BallSpeedDown:
            return "L";
        case BonusType::StickyPaddle:
            return "S";
        case BonusType::SafetyFloor:
            return "D";
        case BonusType::RandomDirection:
            return "R";
        default:
            return "?";
    }
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
        case '+': return {0b00000, 0b00100, 0b00100, 0b11111, 0b00100, 0b00100, 0b00000};
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
    return (x / static_cast<float>(WINDOW_WIDTH)) * 2.0f - 1.0f;
}

float Renderer::screenToNdcY(const float &y) {
    return 1.0f - (y / static_cast<float>(WINDOW_HEIGHT)) * 2.0f;
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

void Renderer::addCircle(std::vector<Vertex> &vertices,
                         const float &center_x,
                         const float &center_y,
                         const float &radius,
                         const Color &color) {
    const int segment_count = 28;
    const float pi = 3.14159265359f;

    float center_ndc_x = screenToNdcX(center_x);
    float center_ndc_y = screenToNdcY(center_y);

    for (int i = 0; i < segment_count; i++) {
        float first_angle = static_cast<float>(i) / static_cast<float>(segment_count) * 2.0f * pi;
        float second_angle = static_cast<float>(i + 1) / static_cast<float>(segment_count) * 2.0f * pi;

        float first_x = screenToNdcX(center_x + std::cos(first_angle) * radius);
        float first_y = screenToNdcY(center_y + std::sin(first_angle) * radius);
        float second_x = screenToNdcX(center_x + std::cos(second_angle) * radius);
        float second_y = screenToNdcY(center_y + std::sin(second_angle) * radius);

        vertices.push_back({center_ndc_x, center_ndc_y, color.r, color.g, color.b});
        vertices.push_back({first_x, first_y, color.r, color.g, color.b});
        vertices.push_back({second_x, second_y, color.r, color.g, color.b});
    }
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

void Renderer::buildFieldVertices(std::vector<Vertex> &vertices) {
    addRectangle(vertices, 0.0f, 0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, {0.07f, 0.08f, 0.11f});
    addRectangle(vertices, FIELD_LEFT - 3.0f, FIELD_TOP - 3.0f, FIELD_WIDTH + 6.0f, 3.0f, {0.25f, 0.27f, 0.34f});
    addRectangle(vertices, FIELD_LEFT - 3.0f, FIELD_TOP - 3.0f, 3.0f, FIELD_HEIGHT + 6.0f, {0.25f, 0.27f, 0.34f});
    addRectangle(vertices, FIELD_RIGHT, FIELD_TOP - 3.0f, 3.0f, FIELD_HEIGHT + 6.0f, {0.25f, 0.27f, 0.34f});
    addRectangle(vertices, FIELD_LEFT - 3.0f, FIELD_BOTTOM, FIELD_WIDTH + 6.0f, 3.0f, {0.12f, 0.13f, 0.18f});
}

void Renderer::buildBrickVertices(const Board &board, std::vector<Vertex> &vertices) {
    for (int row = 0; row < BRICK_ROWS; row++) {
        for (int col = 0; col < BRICK_COLS; col++) {
            const Brick &brick = board.getBrick(row, col);

            if (brick.type == BrickType::Empty) {
                continue;
            }

            Rect rect = Board::getBrickScreenRect(row, col);
            Color color = getBrickColor(brick);

            addRectangle(vertices, rect.x, rect.y, rect.width, rect.height, {0.02f, 0.02f, 0.03f});
            addRectangle(vertices, rect.x + 2.0f, rect.y + 2.0f, rect.width - 4.0f, rect.height - 4.0f, color);

            if (brick.type == BrickType::Unbreakable) {
                addText(vertices, "X", rect.x + rect.width * 0.5f - 7.0f, rect.y + 6.0f, 3.0f, {0.92f, 0.92f, 0.96f});
            } else {
                addText(vertices, std::to_string(brick.health), rect.x + rect.width * 0.5f - 7.0f, rect.y + 6.0f, 3.0f, {0.10f, 0.10f, 0.12f});
            }
        }
    }
}

void Renderer::buildPaddleVertices(const Paddle &paddle, std::vector<Vertex> &vertices) {
    addRectangle(vertices, paddle.x, paddle.y, paddle.width, paddle.height, {0.18f, 0.78f, 0.88f});
    addRectangle(vertices, paddle.x + 8.0f, paddle.y + 4.0f, paddle.width - 16.0f, 3.0f, {0.82f, 0.95f, 1.0f});
}

void Renderer::buildBallVertices(const Ball &ball, std::vector<Vertex> &vertices) {
    addCircle(vertices, ball.position.x, ball.position.y, ball.radius, {0.96f, 0.96f, 0.88f});
}

void Renderer::buildBonusVertices(const Board &board, std::vector<Vertex> &vertices) {
    for (const Bonus &bonus: board.getBonuses()) {
        if (!bonus.active) {
            continue;
        }

        Color color = getBonusColor(bonus.type);
        addRectangle(vertices, bonus.rect.x, bonus.rect.y, bonus.rect.width, bonus.rect.height, color);
        addText(vertices, getBonusText(bonus.type), bonus.rect.x + 6.0f, bonus.rect.y + 3.0f, 2.0f, {0.04f, 0.04f, 0.05f});
    }
}

void Renderer::buildInterfaceVertices(std::vector<Vertex> &vertices,
                                      const int &score,
                                      const int &lives,
                                      const bool &sticky_enabled,
                                      const bool &safety_floor_enabled,
                                      const bool &random_turn_enabled,
                                      const std::string &state_text,
                                      const Color &state_color) {
    Color text_color = {0.82f, 0.84f, 0.90f};

    addText(vertices, "ARKANOID", 42.0f, 20.0f, 3.0f, {0.92f, 0.92f, 0.96f});
    addText(vertices, "SCORE:" + std::to_string(score), 250.0f, 22.0f, 2.4f, text_color);
    addText(vertices, "LIVES:" + std::to_string(lives) + "/" + std::to_string(MAX_LIVES), 430.0f, 22.0f, 2.4f, text_color);

    addText(vertices, "STICKY:" + std::string(sticky_enabled ? "ON" : "OFF"), 42.0f, 662.0f, 2.1f, text_color);
    addText(vertices, "FLOOR:" + std::string(safety_floor_enabled ? "ON" : "OFF"), 215.0f, 662.0f, 2.1f, text_color);
    addText(vertices, "RANDOM:" + std::string(random_turn_enabled ? "ON" : "OFF"), 370.0f, 662.0f, 2.1f, text_color);
    addText(vertices, state_text, 655.0f, 662.0f, 2.1f, state_color);
}

void Renderer::drawVertices(const std::vector<Vertex> &vertices) {
    if (vertices.empty()) {
        return;
    }

    glUseProgram(this->m_shader_program);
    glBindVertexArray(this->m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<long>(vertices.size() * sizeof(Vertex)),
                 vertices.data(),
                 GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(vertices.size()));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Renderer::renderGame(const Board &board,
                          const Paddle &paddle,
                          const Ball &ball,
                          const int &score,
                          const int &lives,
                          const bool &sticky_enabled,
                          const bool &safety_floor_enabled,
                          const bool &random_turn_enabled,
                          const std::string &state_text,
                          const Color &state_color) {
    std::vector<Vertex> vertices;
    vertices.reserve(9000);

    buildFieldVertices(vertices);
    buildBrickVertices(board, vertices);
    buildBonusVertices(board, vertices);
    buildPaddleVertices(paddle, vertices);
    buildBallVertices(ball, vertices);
    buildInterfaceVertices(vertices,
                           score,
                           lives,
                           sticky_enabled,
                           safety_floor_enabled,
                           random_turn_enabled,
                           state_text,
                           state_color);

    if (safety_floor_enabled) {
        addRectangle(vertices, FIELD_LEFT, FIELD_BOTTOM - 6.0f, FIELD_WIDTH, 4.0f, {0.20f, 0.85f, 0.78f});
    }

    drawVertices(vertices);
}
