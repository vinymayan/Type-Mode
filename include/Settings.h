#include <Windows.h>
#include <map>
#include <string>
#include "SKSEMCP/SKSEMenuFramework.hpp"
#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/writer.h"

namespace Settings {
    // Checkboxes (baseado no seu MCM)
    inline bool TypeMode = true;
    inline bool ShowLogs = true;

    // Keybinds (separados para teclado e controle)
    // Os valores padrão podem ser ajustados conforme necessário. Usei códigos de tecla comuns.
    inline uint32_t TypeMode_k = 62;  // Teclado (E)
    inline uint32_t TypeMode_m = 0;
    inline uint32_t TypeMode_g = 0;  // Controle (LB / L1)
    inline std::map<std::string, std::set<uint32_t>> MenuExceptions;
}

// Namespace para organizar as funções do nosso menu
namespace Menu {
    // Registra o menu no SKSE Menu Framework
    void Register();

    // Funções para salvar e carregar as configurações de um arquivo JSON
    void LoadSettings();
    void SaveSettings();
}

const std::map<int, const char*> g_gamepad_dx_to_name_map = {
    // --- GAMEPAD (ATUALIZADO) ---
    {0, "[Nenhuma]"},
    {1, "DPad Up"},
    {2, "DPad Down"},
    {4, "DPad Left"},
    {8, "DPad Right"},
    {16, "Start"},
    {32, "Back"},
    {64, "L3"},
    {128, "R3"},
    {256, "LB"},
    {512, "RB"},
    {4096, "A / X"},
    {8192, "B / O"},
    {16384, "X / Square"},
    {32768, "Y / Triangle"},
    {9, "LT/L2"},
    {10, "RT/R2"}};

const std::map<int, const char*> g_dx_to_name_map = {
    {0, "[Nenhuma]"},
    {1, "Escape"},
    {2, "1"},
    {3, "2"},
    {4, "3"},
    {5, "4"},
    {6, "5"},
    {7, "6"},
    {8, "7"},
    {9, "8"},
    {10, "9"},
    {11, "0"},
    {12, "-"},
    {13, "="},
    {14, "Backspace"},
    {15, "Tab"},
    {16, "Q"},
    {17, "W"},
    {18, "E"},
    {19, "R"},
    {20, "T"},
    {21, "Y"},
    {22, "U"},
    {23, "I"},
    {24, "O"},
    {25, "P"},
    {26, "["},          // <-- NOVA (Abre Colchetes)
    {27, "]"},          // <-- NOVA (Fecha Colchetes)
    {28, "Enter"},
    {29, "Left Ctrl"},
    {30, "A"},
    {31, "S"},
    {32, "D"},
    {33, "F"},
    {34, "G"},
    {35, "H"},
    {36, "J"},
    {37, "K"},
    {38, "L"},
    {39, ";"},
    {40, "'"},          // <-- NOVA (Aspas simples)
    {41, "`"},          // <-- NOVA (Crase/Tilde)
    {42, "Left Shift"},
    {43, "\\"},
    {44, "Z"},
    {45, "X"},
    {46, "C"},
    {47, "V"},
    {48, "B"},
    {49, "N"},
    {50, "M"},
    {51, ","},
    {52, "."},
    {53, "/"},
    {54, "Right Shift"},
    {56, "Left Alt"},
    {57, "Spacebar"},
    {58, "Caps Lock"},  // <-- NOVA (A que você pediu)
    {59, "F1"},
    {60, "F2"},
    {61, "F3"},
    {62, "F4"},
    {63, "F5"},
    {64, "F6"},
    {65, "F7"},
    {66, "F8"},
    {67, "F9"},
    {68, "F10"},
    {69, "Num Lock"},    // <-- NOVA
    {70, "Scroll Lock"}, // <-- NOVA
    {87, "F11"},
    {88, "F12"},
    {156, "Keypad Enter"},
    {157, "Right Ctrl"},
    {183, "Print Screen"}, // <-- NOVA (Geralmente SysRq)
    {184, "Right Alt"},
    {197, "Pause"},        // <-- NOVA
    {199, "Home"},
    {200, "Up Arrow"},
    {201, "PgUp"},
    {203, "Left Arrow"},
    {205, "Right Arrow"},
    {207, "End"},
    {208, "Down Arrow"},
    {209, "PgDown"},
    {210, "Insert"},
    {211, "Delete"},
    {219, "Left Windows"},  // <-- NOVA
    {220, "Right Windows"}, // <-- NOVA
    {221, "Apps Menu"},     // <-- NOVA
    //{256, "Left Click"},
    {257, "Right Click"},
    {258, "Middle Mouse Button"},
    {259, "Mouse 4"},
    {260, "Mouse 5"},
    {261, "Mouse 6"},
    {262, "Mouse 7"},
    {263, "Mouse 8"},
    // Teclado Numérico (Keypad)
    {55, "Keypad *"},
    {181, "Keypad /"},
    {74, "Keypad -"},
    {78, "Keypad +"},
    {71, "Keypad 7"},
    {72, "Keypad 8"},
    {73, "Keypad 9"},
    {75, "Keypad 4"},
    {76, "Keypad 5"},
    {77, "Keypad 6"},
    {79, "Keypad 1"},
    {80, "Keypad 2"},
    {81, "Keypad 3"},
    {82, "Keypad 0"},
    {83, "Keypad ."}
};