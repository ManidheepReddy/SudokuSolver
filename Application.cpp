// A self‑contained Sudoku Solver using C++, OpenGL, GLFW, GLEW, and ImGui v1.60

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"


class SudokuSolver {
public:
    bool solve(std::string& board) {
        if (board.size() != 81 || !isValid(board)) return false;
        return backtrack(board);
    }

private:
    bool backtrack(std::string& b) {
        auto pos = b.find('-');
        if (pos == std::string::npos) return true;            

        int idx = int(pos);
        auto choices = nextChoices(b, idx);
        for (char d : choices) {
            b[idx] = d;
            if (backtrack(b)) return true;
        }
        b[idx] = '-';                                        
        return false;
    }

    bool isValid(const std::string& b) {
        return allUnitsValid(b, 0, 9, 1)  
            && allUnitsValid(b, 0, 9, 9)  
            && allBoxesValid(b);
    }

    bool allUnitsValid(const std::string& b, int start, int count, int step) {
        for (int i = 0; i < count; i++) {
            std::unordered_set<char> seen;
            for (int j = 0; j < count; j++) {
                char c = b[start + i * step + j * (step == 9 ? 1 : 9)];
                if (c != '-' && !seen.insert(c).second)
                    return false;
            }
        }
        return true;
    }

    bool allBoxesValid(const std::string& b) {
        int boxStarts[9] = { 0,3,6,27,30,33,54,57,60 };
        for (int s : boxStarts) {
            std::unordered_set<char> seen;
            for (int dy = 0; dy < 3; dy++)
                for (int dx = 0; dx < 3; dx++) {
                    char c = b[s + dy * 9 + dx];
                    if (c != '-' && !seen.insert(c).second)
                        return false;
                }
        }
        return true;
    }

    std::vector<char> nextChoices(const std::string& b, int idx) {
        bool used[10] = {};
        int r = idx / 9, c = idx % 9;
        // row & column
        for (int i = 0; i < 9; i++) {
            char cr = b[r * 9 + i];
            char cc = b[i * 9 + c];
            if (cr != '-') used[cr - '0'] = true;
            if (cc != '-') used[cc - '0'] = true;
        }
        // box
        int br = (r / 3) * 3, bc = (c / 3) * 3;
        for (int dy = 0; dy < 3; dy++)
            for (int dx = 0; dx < 3; dx++) {
                char cb = b[(br + dy) * 9 + (bc + dx)];
                if (cb != '-') used[cb - '0'] = true;
            }
        // collect
        std::vector<char> v;
        for (char d = '1'; d <= '9'; d++)
            if (!used[d - '0']) v.push_back(d);
        return v;
    }
};


static std::string boardToString(char grid[9][9][2]) {
    std::string s; s.reserve(81);
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            s += (grid[r][c][0] >= '1' && grid[r][c][0] <= '9')
            ? grid[r][c][0] : '-';
    return s;
}

static void stringToBoard(char grid[9][9][2], const std::string& s) {
    for (int i = 0; i < 81; i++) {
        grid[i / 9][i % 9][0] = s[i];
        grid[i / 9][i % 9][1] = '\0';
    }
}

static void clearBoard(char grid[9][9][2]) {
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            grid[r][c][0] = grid[r][c][1] = '\0';
}


int main() {
    // GLFW + OpenGL init
    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window =
        glfwCreateWindow(960, 540, "Sudoku Solver", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) {
        std::cerr << "[GLEW] init failed\n";
        return -1;
    }

    // ImGui v1.60 init
    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();

    char grid[9][9][2] = { {0} };
    SudokuSolver solver;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();

        // Clear background
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Sudoku window
        ImGui::SetNextWindowPos({ 210, 0 });
        ImGui::SetNextWindowSize({ 540, 600 });
        ImGui::Begin("Sudoku", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);

       
        auto dl = ImGui::GetWindowDrawList();
        auto p = ImGui::GetWindowPos();
        for (int i = 0; i <= 9; i++) {
            float x = p.x + i * 60, y = p.y + i * 60;
            dl->AddLine({ x,p.y }, { x,p.y + 540 }, IM_COL32(0, 0, 0, 255));
            dl->AddLine({ p.x,y }, { p.x + 540,y }, IM_COL32(0, 0, 0, 255));
        }

      
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                ImGui::SetCursorPos({ float(c * 60 + 5),
                                     float(r * 60 + 5) });
                ImGui::PushID(r * 9 + c);
                ImGui::PushItemWidth(50);
                ImGui::InputText("##cell",
                    grid[r][c], 2,
                    ImGuiInputTextFlags_CharsDecimal |
                    ImGuiInputTextFlags_AutoSelectAll);
                ImGui::PopItemWidth();
                ImGui::PopID();
            }
        }

       
        ImGui::SetCursorPos({ 10, 550 });
        if (ImGui::Button("Solve")) {
            std::string s = boardToString(grid);
            if (solver.solve(s))
                stringToBoard(grid, s);
            else
                ImGui::OpenPopup("Invalid Board");
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            clearBoard(grid);
        }

     
        if (ImGui::BeginPopupModal("Invalid Board", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Invalid or unsolvable puzzle!");
            if (ImGui::Button("OK"))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::End();              
       
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }


    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
