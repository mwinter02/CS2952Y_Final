//
// Created by Marcus Winter on 11/19/25.
//

#include "UI.h"

#include <string>


#include "Debug.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "nfd.h"



/**
 * Initialize ImGui UI for the given GLFW window.
 * @param window - Pointer to the GLFW window.
 */
void UI::initialize(GLFWwindow* window) {
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410 core");
    ImGui::StyleColorsClassic();
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg].w = 0.7f;
}


void UI::beginDraw(float x_pos, float y_pos, float width, float height) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos({x_pos, y_pos});
    ImGui::SetNextWindowSize({width,height});
}

void UI::endDraw() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

std::string UI::openFileExplorer(std::vector<FileFilters> file_filters) {
    NFD_Init();

    nfdu8filteritem_t filters[file_filters.size()];
    for (int i=0; i<file_filters.size(); i++) {
        filters[i].name = file_filters[i].name.c_str();
        filters[i].spec = file_filters[i].spec.c_str();
    }

    nfdu8char_t *outPath;
    nfdopendialogu8args_t args = {0};
    args.filterList = filters;
    args.filterCount = file_filters.size();
    nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

    std::string out_string;
    if (result == NFD_OKAY)
    {
        out_string = std::string(outPath);
        NFD_FreePathU8(outPath);
    }
    else if (result == NFD_CANCEL)
    {
        puts("User pressed cancel.");
        outPath = nullptr;
    }
    else
    {
        printf("Error: %s\n", NFD_GetError());
    }

    NFD_Quit();

    return out_string;
}


