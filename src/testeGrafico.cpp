#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

int main()
{
    InitWindow(1000, 650, "Simulador de SO");
    SetTargetFPS(60);

    rlImGuiSetup(true);

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(Color{30, 34, 42, 255});

        rlImGuiBegin();

        ImGui::SetNextWindowSize(
            ImVec2(400, 220),
            ImGuiCond_FirstUseEver
        );

        ImGui::Begin("Simulador de SO");

        ImGui::Text("Interface grafica funcionando!");

        static int tick = 0;

        ImGui::Text("Tick atual: %d", tick);

        if (ImGui::Button("Proximo tick"))
        {
            tick++;
        }

        ImGui::SameLine();

        if (ImGui::Button("Resetar"))
        {
            tick = 0;
        }

        ImGui::End();

        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow();

    return 0;
}