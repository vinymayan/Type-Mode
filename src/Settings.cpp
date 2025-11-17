#include "Settings.h"

#include "SKSEMCP/SKSEMenuFramework.hpp"


namespace Menu {

    // Caminho para o nosso arquivo de configurações
    const char* SETTINGS_PATH = "Data/SKSE/Plugins/TypeMode_Settings.json";

    // Função auxiliar para criar um seletor de teclas (ComboBox)
    // Baseado no seu exemplo Events.cpp
    void RenderKeybind(const char* label, uint32_t* key_k, uint32_t* key_m, uint32_t* key_g, bool mouse_only = false) {
        bool settings_changed = false;

        // --- DROPDOWN UNIFICADO PARA TECLADO E MOUSE ---
        ImGui::Text("%s (PC)", label);
        ImGui::SameLine();

        // 1. Descobre qual tecla (teclado ou mouse) está atualmente selecionada.
        uint32_t current_pc_key = (*key_k != 0) ? *key_k : *key_m;

        // 2. Busca o nome da tecla atual para exibir no dropdown fechado.
        const char* current_key_name_pc = "[Nenhuma]";
        auto it_pc = g_dx_to_name_map.find(current_pc_key);
        if (it_pc != g_dx_to_name_map.end()) {
            current_key_name_pc = it_pc->second;
        }

        // 3. Cria o ComboBox.
        if (ImGui::BeginCombo((std::string("##_pc_") + label).c_str(), current_key_name_pc)) {
            // Itera por TODAS as teclas (teclado e mouse) no mapa.
            for (auto const& [key_code, key_name] : g_dx_to_name_map) {
                if (mouse_only && key_code < 256) continue;
                const bool is_selected = (current_pc_key == key_code);
                if (ImGui::Selectable(key_name, is_selected)) {
                    // 4. LÓGICA DE EXCLUSÃO MÚTUA
                    // Se a tecla selecionada for do teclado (< 256)...
                    if (key_code < 256) {
                        *key_k = key_code;  // Define a tecla do teclado.
                        *key_m = 0;         // Limpa a tecla do mouse.
                    } else {                // Senão, é uma tecla do mouse.
                        *key_m = key_code;  // Define a tecla do mouse.
                        *key_k = 0;         // Limpa a tecla do teclado.
                    }
                    settings_changed = true;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        // --- CONTROLE ---
        const char* current_key_name_g = "[Nenhuma]";  // Valor padrão
        auto it_g = g_gamepad_dx_to_name_map.find(*key_g);
        if (it_g != g_gamepad_dx_to_name_map.end()) {
            current_key_name_g = it_g->second;
        }

        ImGui::Text("%s (Gamepad)", label);
        ImGui::SameLine();
        if (ImGui::BeginCombo((std::string("##_g_") + label).c_str(), current_key_name_g)) {
            for (auto const& [key_code, key_name] : g_gamepad_dx_to_name_map) {
                const bool is_selected = (*key_g == key_code);
                if (ImGui::Selectable(key_name, is_selected)) {
                    *key_g = key_code;
                    settings_changed = true;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        if (settings_changed) {
            SaveSettings();
        }
    }

    // Função principal que desenha o conteúdo do menu (sem alterações necessárias aqui)
    void __stdcall Render() {
        bool settings_changed = false;

        ImGui::Text("Typing Mode");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Checkbox("Enable Type mode", &Settings::TypeMode)) settings_changed = true;
        RenderKeybind("Type mode Key", &Settings::TypeMode_k, &Settings::TypeMode_m, &Settings::TypeMode_g);

        if (settings_changed) {
            SaveSettings();
        }
    }


    // Salva as configurações em JSON
    void SaveSettings() {
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        doc.AddMember("TypeMode", Settings::TypeMode, allocator);
        doc.AddMember("TypeMode_k", Settings::TypeMode_k, allocator);
        doc.AddMember("TypeMode_m", Settings::TypeMode_m, allocator);
        doc.AddMember("TypeMode_g", Settings::TypeMode_g, allocator);


        FILE* fp = nullptr;
        fopen_s(&fp, SETTINGS_PATH, "wb");
        if (fp) {
            char writeBuffer[65536];
            rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
            rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
            doc.Accept(writer);
            fclose(fp);
        }

    }

    // Carrega as configurações do JSON
    void LoadSettings() {
        FILE* fp = nullptr;
        fopen_s(&fp, SETTINGS_PATH, "rb");
        if (fp) {
            char readBuffer[65536];
            rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
            rapidjson::Document doc;
            doc.ParseStream(is);
            fclose(fp);

            if (doc.IsObject()) {
                if (doc.HasMember("TypeMode") && doc["TypeMode"].IsBool())
                    Settings::TypeMode = doc["TypeMode"].GetBool();
                if (doc.HasMember("TypeMode_k") && doc["TypeMode_k"].IsInt())
                    Settings::TypeMode_k = doc["TypeMode_k"].GetInt();
                if (doc.HasMember("TypeMode_m") && doc["TypeMode_m"].IsInt())
                    Settings::TypeMode_m = doc["TypeMode_m"].GetInt();
                if (doc.HasMember("TypeMode_g") && doc["TypeMode_g"].IsInt())
                    Settings::TypeMode_g = doc["TypeMode_g"].GetInt();


            }
        }

    }

    // Registra o menu
    void Register() {
        if (SKSEMenuFramework::IsInstalled()) {
            SKSE::log::info("SKSE Menu Framework encontrado. Registrando o menu Type Mode.");

            SKSEMenuFramework::SetSection("Type Mode");
            SKSEMenuFramework::AddSectionItem("Settings", Render);
        } else {
            SKSE::log::warn("SKSE Menu Framework nao encontrado. O menu Type Mode nao sera registrado.");
        }
    }
}