#include "Settings.h"

#include "SKSEMCP/SKSEMenuFramework.hpp"

namespace Menu {

    // Caminho para o nosso arquivo de configurações
    const char* SETTINGS_PATH = "Data/SKSE/Plugins/TypeMode_Settings.json";
    static char tempMenuName[128] = "";
    static std::string menuToEditKeyStr = "";
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
        bool should_open_popup = false;

        ImGui::Text("Typing Mode");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Checkbox("Enable Typing mode", &Settings::TypeMode)) settings_changed = true;
        RenderKeybind("Typing mode Key", &Settings::TypeMode_k, &Settings::TypeMode_m, &Settings::TypeMode_g);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Menu Input Exceptions (Unblocked Keys)");
        ImGui::TextWrapped(
            "Add menus to be auto block and which keys should be whitelisted to open a menu");

        // Input para adicionar novo menu
        ImGui::InputText("Menu name", tempMenuName, sizeof(tempMenuName));
        ImGui::SameLine();
        if (ImGui::Button("Add menu")) {
            if (strlen(tempMenuName) > 0) {
                std::string sName(tempMenuName);
                if (Settings::MenuExceptions.find(sName) == Settings::MenuExceptions.end()) {
                    Settings::MenuExceptions[sName] = {};  // Cria entrada vazia
                    settings_changed = true;
                }
            }
        }

        ImGui::Spacing();

        // TABELA DE EXCEÇÕES
        if (ImGui::BeginTable("ExceptionsTable", 3,
                              ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Menu Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Unblocked Keys", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableHeadersRow();

            // Itera sobre o mapa para preencher a tabela.
            // Usamos iteradores para poder deletar itens seguramente.
            for (auto it = Settings::MenuExceptions.begin(); it != Settings::MenuExceptions.end();) {
                ImGui::PushID(it->first.c_str());  // Garante IDs únicos por linha

                ImGui::TableNextRow();

                // Coluna 1: Nome do Menu
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", it->first.c_str());

                // Coluna 2: Teclas Desbloqueadas
                ImGui::TableSetColumnIndex(1);

                // Botão de Adicionar Tecla (+)
                if (ImGui::Button("+")) {
                    menuToEditKeyStr = it->first;
                    should_open_popup = true;
                }
                ImGui::SameLine();

                // Lista as teclas atuais como pequenos botões ou texto
                for (auto keyIt = it->second.begin(); keyIt != it->second.end();) {
                    const char* kName = "Unknown";
                    auto mapIt = g_dx_to_name_map.find(*keyIt);
                    if (mapIt != g_dx_to_name_map.end()) kName = mapIt->second;

                    // Exibe a tecla. Se clicar nela, remove? Ou adiciona um 'x' pequeno.
                    // Vamos fazer um botão pequeno com o nome. Clicar remove.
                    char btnLabel[64];
                    sprintf_s(btnLabel, "%s [x]", kName);
                    if (ImGui::SmallButton(btnLabel)) {
                        keyIt = it->second.erase(keyIt);
                        settings_changed = true;
                    } else {
                        ImGui::SameLine();
                        ++keyIt;
                    }
                }

                // Coluna 3: Remover Linha inteira
                ImGui::TableSetColumnIndex(2);
                if (ImGui::Button("Remove")) {
                    it = Settings::MenuExceptions.erase(it);
                    settings_changed = true;
                } else {
                    ++it;
                }

                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        if (should_open_popup) {
            ImGui::OpenPopup("SelectKeyPopup");
        }
        // POPUP PARA SELECIONAR TECLA
        if (ImGui::BeginPopup("SelectKeyPopup")) {
            ImGui::Text("Select the key: %s", menuToEditKeyStr.c_str());
            ImGui::Separator();

            // Filtro ou Lista simples
            ImGui::BeginChild("KeyList", ImVec2(300, 400));
            for (auto const& [key_code, key_name] : g_dx_to_name_map) {
                if (ImGui::Selectable(key_name,false)) {
                    Settings::MenuExceptions[menuToEditKeyStr].insert(key_code);
                    settings_changed = true;
                    ImGui::CloseCurrentPopup();
                }
            }
            ImGui::EndChild();
            ImGui::EndPopup();
        }

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

        rapidjson::Value exceptionsArray(rapidjson::kArrayType);
        for (const auto& [menuName, keys] : Settings::MenuExceptions) {
            rapidjson::Value entryObj(rapidjson::kObjectType);

            rapidjson::Value nameVal;
            nameVal.SetString(menuName.c_str(), allocator);
            entryObj.AddMember("MenuName", nameVal, allocator);

            rapidjson::Value keysArray(rapidjson::kArrayType);
            for (uint32_t k : keys) {
                keysArray.PushBack(k, allocator);
            }
            entryObj.AddMember("UnblockedKeys", keysArray, allocator);

            exceptionsArray.PushBack(entryObj, allocator);
        }
        doc.AddMember("MenuExceptions", exceptionsArray, allocator);

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
                if (doc.HasMember("TypeMode")) Settings::TypeMode = doc["TypeMode"].GetBool();
                if (doc.HasMember("TypeMode_k")) Settings::TypeMode_k = doc["TypeMode_k"].GetInt();
                if (doc.HasMember("TypeMode_m")) Settings::TypeMode_m = doc["TypeMode_m"].GetInt();
                if (doc.HasMember("TypeMode_g")) Settings::TypeMode_g = doc["TypeMode_g"].GetInt();

                // Carrega Tabela de Exceções
                Settings::MenuExceptions.clear();
                if (doc.HasMember("MenuExceptions") && doc["MenuExceptions"].IsArray()) {
                    for (const auto& entry : doc["MenuExceptions"].GetArray()) {
                        if (entry.HasMember("MenuName") && entry.HasMember("UnblockedKeys")) {
                            std::string mName = entry["MenuName"].GetString();
                            std::set<uint32_t> keys;
                            for (const auto& k : entry["UnblockedKeys"].GetArray()) {
                                keys.insert(k.GetUint());
                            }
                            Settings::MenuExceptions[mName] = keys;
                        }
                    }
                }
            }
        
        }

    }

    // Registra o menu
    void Register() {
        if (SKSEMenuFramework::IsInstalled()) {
            SKSE::log::info("SKSE Menu Framework encontrado. Registrando o menu Type Mode.");

            SKSEMenuFramework::SetSection("Typing Mode");
            SKSEMenuFramework::AddSectionItem("Settings", Render);
        } else {
            SKSE::log::warn("SKSE Menu Framework nao encontrado. O menu Type Mode nao sera registrado.");
        }
    }
}