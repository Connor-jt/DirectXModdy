#pragma once

static bool skip_blockies = false;

// config settings stuff
unsigned long long camera_rotation_yaw_offset = 0;
unsigned long long camera_rotation_pit_offset = 0;
unsigned long long camera_rotation_rol_offset = 0;

unsigned long long camera_position_x_offset = 0;
unsigned long long camera_position_y_offset = 0;
unsigned long long camera_position_z_offset = 0;

static char target_window_name[256] = {};

// these should be per object values !!!!!
static float w_scale = 1.0f;

static bool rotation_relative;
static bool position_relative;

static bool invert_x = false;
static bool invert_y = false;
static bool invert_z = false;

static float w_yaw;
static float w_pitch;
static float w_roll;

static float w_x;
static float w_y;
static float w_z;

static float fov = 84;
float4x4 __perspectiveMat = {};
// cleanup
ID3D11ShaderResourceView* myTexture = 0;



bool IsValidHex(const char* str) {
    while (*str)
        if (!std::isxdigit(*str)) return false;
        else str++;
    return true;
}
void Uint64Field(const char* label, unsigned long long* variable) {
    static char input[17] = ""; // Buffer for 16 hex digits + null terminator
    sprintf_s(input, "%016llX", *variable); // fill text buffer with value
    // display label, and if changed then apply
    if (ImGui::InputText(label, input, sizeof(input), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase)
        && IsValidHex(input))
        *variable = std::strtoull(input, nullptr, 16);
}

void RegeneratePerspectiveMatrix(HWND hwnd) {
    // Get window dimensions
    int windowWidth, windowHeight;
    float windowAspectRatio;
    {
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        windowWidth = clientRect.right - clientRect.left;
        windowHeight = clientRect.bottom - clientRect.top;
        windowAspectRatio = (float)windowWidth / (float)windowHeight;
    }
    __perspectiveMat = makePerspectiveMat(windowAspectRatio, degreesToRadians(fov), 0.1f, 1000.f);
}

void DrawGUI(ID3D11Device1* dx_device, ID3D11DeviceContext1* dx_device_context, HWND target_hwnd) {
    // reload target window thing
    if (myTexture) myTexture->Release();
    myTexture = GetTargetProcessScreen(dx_device_context, dx_device, target_window_name);

    std::vector<float4x4> views = {};

    drawcount = 0;
    float4x4 modelViewProj;
    if (rotation_relative) {

        // validate camera offsets
        float temp_buffer;
        if (!ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_rotation_yaw_offset, &temp_buffer, 4, NULL)
            || !ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_rotation_pit_offset, &temp_buffer, 4, NULL)
            || !ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_rotation_rol_offset, &temp_buffer, 4, NULL))
        {
            drawcount = 1;
            goto data_offset_failed;
        }

        {   // get widget size
            RECT clientRect;
            GetClientRect(target_hwnd, &clientRect);
            float widget_size = 5.0f / (clientRect.right - clientRect.left);
            widget_size *= w_scale;

            if (!position_relative) {
                drawcount = 2;
                float3 widget_camera_position = { 0, 0, 0 };
                // NOTE: cheapo rotation logic that doesn't actually work right!!!!
                float4x4 widget_viewMat = translationMat(-widget_camera_position) * rotateYMat(-*(float*)camera_rotation_yaw_offset + w_yaw) * rotateXMat(-*(float*)camera_rotation_pit_offset + w_pitch);
                float4x4 widget_matrix = scaleMat(float3{ widget_size, -widget_size, -widget_size }) * translationMat({ 0, 0, -1.0f });
                modelViewProj = widget_matrix * widget_viewMat * __perspectiveMat;

            }
            else { // draw into world space

                if (!ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_position_x_offset, &temp_buffer, 4, NULL)
                    || !ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_position_y_offset, &temp_buffer, 4, NULL)
                    || !ReadProcessMemory(GetCurrentProcess(), (LPCVOID)camera_position_z_offset, &temp_buffer, 4, NULL))
                    { drawcount = 3; goto data_offset_failed;  } {
                    

                    drawcount = 4;
                    float x = (invert_x) ? -*(float*)camera_position_x_offset : *(float*)camera_position_x_offset;
                    float y = (invert_y) ? -*(float*)camera_position_y_offset : *(float*)camera_position_y_offset;
                    float z = (invert_z) ? -*(float*)camera_position_z_offset : *(float*)camera_position_z_offset;

                    float3 widget_camera_position = { x, y, z };
                    float4x4 widget_viewMat = translationMat(-widget_camera_position) * rotateYMat(-*(float*)camera_rotation_yaw_offset) * rotateXMat(-*(float*)camera_rotation_pit_offset);
                    float4x4 widget_matrix = scaleMat(float3{ widget_size, -widget_size, -widget_size }) * rotateYMat(w_yaw) * rotateXMat(w_pitch) * translationMat({ w_x, w_y, w_z });
                    modelViewProj = widget_matrix * widget_viewMat * __perspectiveMat;

                }
            }
        }
    }
    else data_offset_failed:
    modelViewProj = nanMat();


    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(544, 576));
    ImGui::Begin("Sample copy window", NULL, ImGuiWindowFlags_NoInputs);
    //ImGui::Begin("Sample copy window");
    views.push_back(modelViewProj);

    if (myTexture)
        ImGui::Image((ImTextureID)myTexture, ImVec2(512, 512));
    else ImGui::Text("No image loaded!");


    views.push_back(modelViewProj);
    ImGui::End();


    ImGui::Begin("Config UIs");
    ImGui::InputText("Target process", target_window_name, 256);
    if (ImGui::DragFloat("UI FOV", &fov, 1, 45, 180)) 
        RegeneratePerspectiveMatrix(target_hwnd);

    ImGui::Checkbox("Dont draw ugly cubes", &skip_blockies);
    ImGui::Checkbox("Camera rotation relative", &rotation_relative);
    if (rotation_relative) {
        Uint64Field("Camera YAW", &camera_rotation_yaw_offset);
        Uint64Field("Camera PITCH", &camera_rotation_pit_offset);
        Uint64Field("Camera ROLL", &camera_rotation_rol_offset);

        ImGui::DragFloat("Widget YAW", &w_yaw, 0.01f);
        ImGui::DragFloat("Widget PITCH", &w_pitch, 0.01f);
        ImGui::DragFloat("Widget ROLL", &w_roll, 0.01f);
        ImGui::DragFloat("Widget SCALE", &w_scale, 0.025f, 0.01f);

        ImGui::Checkbox("Camera position relative", &position_relative);
        if (position_relative) {
            Uint64Field("Camera X", &camera_position_x_offset);
            Uint64Field("Camera Y", &camera_position_y_offset);
            Uint64Field("Camera Z", &camera_position_z_offset);
            ImGui::Checkbox("Invert Camera X", &invert_x);
            ImGui::Checkbox("Invert Camera Y", &invert_y);
            ImGui::Checkbox("Invert Camera Z", &invert_z);

            ImGui::DragFloat("Widget X", &w_x, 0.025f);
            ImGui::DragFloat("Widget Y", &w_y, 0.025f);
            ImGui::DragFloat("Widget Z", &w_z, 0.025f);

        }
    }
    ImGui::End();
    views.push_back(nanMat());

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData(), views, (unsigned long long)target_hwnd);
}