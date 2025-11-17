#include "logger.h"
#include "Settings.h"
#include <set>      // Adicionado para std::set
#include <string>   // Adicionado para std::string

// --- Nossas Variáveis Globais ---
static uintptr_t g_VanillaFuncPtr = 0;  // O endereço que vamos pegar do Plugin 1
static uintptr_t g_NextHookPtr = 0;
static bool g_IsMenuOpen = false;       // Sua flag "testar"

// O tipo da função que estamos hookando
using PollInputDevices_t = void(RE::BSTEventSource<RE::InputEvent*>*, RE::InputEvent**);
static bool g_IsBlockerToggledOn = false;  // Define se o bloqueio está ATIVADO ou DESATIVADO.
    

class MenuOpenCloseListener : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
public:
    // Lista de menus vanilla que queremos rastrear
    // Esta é a mesma lista da sua função IsVanillaMenuOpen
    const std::set<std::string> vanillaMenus = {
        "Console",      "Dialogue Menu", "Crafting Menu",    "FavoritesMenu", "GiftMenu",  "InventoryMenu",
        "Journal Menu", "LevelUp Menu",  "Lockpicking Menu", "MagicMenu",     "MapMenu",   "MessageBoxMenu",
        "StatsMenu",    "TweenMenu",     "BarterMenu",       "ContainerMenu", "Book Menu", "SleepWaitMenu"};

    // Rastreia quantos menus vanilla estão abertos
    // Usamos static inline para que seja partilhado e inicializado
    static inline std::atomic<int> vanillaMenuCount = 0;

    static MenuOpenCloseListener* GetSingleton() {
        static MenuOpenCloseListener singleton;
        return &singleton;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::MenuOpenCloseEvent* a_event,
                                          RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
        if (a_event && a_event->menuName.c_str()) {
            // Verifique se o menu do evento está na nossa lista de vanilla
            if (vanillaMenus.count(a_event->menuName.c_str()) > 0) {
                if (a_event->opening) {
                    vanillaMenuCount++;
                } else {
                    // Garante que não fique negativo
                    if (vanillaMenuCount > 0) {
                        vanillaMenuCount--;
                    }
                }
            }
            // Atualize a flag global SOMENTE se um menu vanilla estiver aberto
            g_IsMenuOpen = (vanillaMenuCount > 0);
        }
        return RE::BSEventNotifyControl::kContinue;
    }
};

class AttackStateManager : public RE::BSTEventSink<RE::InputEvent*> {  // TIPO 2: Sem ponteiro aqui
public:
    static AttackStateManager* GetSingleton() {
        static AttackStateManager singleton;
        return &singleton;
    }

    // Assinatura CORRETA para RE::InputEvent*
    RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event,
                                          RE::BSTEventSource<RE::InputEvent*>* a_source) override {
        if (!a_event || !*a_event) {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto player = RE::PlayerCharacter::GetSingleton();

        for (auto event = *a_event; event; event = event->next) {
            if (event->eventType != RE::INPUT_EVENT_TYPE::kButton) {
                continue;
            }

            auto buttonEvent = event->AsButtonEvent();
            if (!buttonEvent) {
                continue;
            }

            auto device = buttonEvent->GetDevice();
            auto keyCode = buttonEvent->GetIDCode();

            if (device == RE::INPUT_DEVICE::kMouse) {
                keyCode += 256;  // Ajusta o código do mouse para corresponder às configurações
            }

            bool isBlockBtnPressed = (device == RE::INPUT_DEVICE::kKeyboard && keyCode == Settings::TypeMode_k) ||
                                     (device == RE::INPUT_DEVICE::kMouse && keyCode == Settings::TypeMode_m) ||
                                     (device == RE::INPUT_DEVICE::kGamepad && keyCode == Settings::TypeMode_g);

            if (isBlockBtnPressed) {
                if (buttonEvent->IsDown()) {
                    g_IsBlockerToggledOn = !g_IsBlockerToggledOn;
                    logger::info("Bloqueio de entrada {}", g_IsBlockerToggledOn ? "ATIVADO" : "DESATIVADO");
                }
                
            }
        }
    }

};

// --- O Hook Principal (O Porteiro) ---
static void hk_BlockerHook(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher, RE::InputEvent** a_events) {
    // Ponteiros de função para os próximos hooks
    auto VanillaFunc = (PollInputDevices_t*)g_VanillaFuncPtr;
    auto NextHookFunc = (PollInputDevices_t*)g_NextHookPtr;
    if (g_IsBlockerToggledOn) {
        return VanillaFunc(a_dispatcher, a_events);
    }

    // Nenhum menu vanilla está aberto. Deixe o Modex (e outros) funcionar normalmente.
    return NextHookFunc(a_dispatcher, a_events);
}


// Helper para pegar o ponteiro do Plugin 1
void GetVanillaPointerFromStealer() {

    HMODULE hStealerMod = GetModuleHandle(L"aaaa_TypeMode.dll");
    if (!hStealerMod) {
        // Falha crítica, o "Ladrão" não carregou
        return;
    }

    // 2. Defina o tipo da função que estamos importando ("GetVanillaFunctionPointer")
    using _GetVanillaFunctionPointer = uintptr_t (*)();

    // 3. Obtenha o endereço da função exportada
    auto GetPointerFunc = (_GetVanillaFunctionPointer)GetProcAddress(hStealerMod, "GetVanillaFunctionPointer");

    if (GetPointerFunc) {
        // 4. Chame a função e salve o ponteiro!
        g_VanillaFuncPtr = GetPointerFunc();
    }
}

static bool g_HookInstalled = false;

void InstallHook() {
    if (g_HookInstalled) {
        return;
    }

    // Pegue o ponteiro vanilla ANTES de instalar o hook
    GetVanillaPointerFromStealer();
    if (g_VanillaFuncPtr == 0) {
        logger::error("Falha ao pegar VanillaFuncPtr. O Bloqueador está desativado.");
        return;
    }

    logger::info("Instalando o hook do Bloqueador...");

    REL::RelocationID hookLocation(67315, 68617);
    uintptr_t hookAddress = hookLocation.address() + REL::Relocate(0x7B, 0x7B, 0x81);

    SKSE::AllocTrampoline(14);
    auto& trampoline = SKSE::GetTrampoline();

    g_NextHookPtr = trampoline.write_call<5>(hookAddress, hk_BlockerHook);

    logger::info("Hook do Bloqueador instalado.");
    g_HookInstalled = true;
}

void OnMessage(SKSE::MessagingInterface::Message* message) {
    if (message->type == SKSE::MessagingInterface::kDataLoaded) {
        RE::UI::GetSingleton()->AddEventSink(MenuOpenCloseListener::GetSingleton());
        Menu::LoadSettings();
        Menu::Register();
    }
    if (message->type == SKSE::MessagingInterface::kNewGame || message->type == SKSE::MessagingInterface::kPostLoadGame) {
        InstallHook();
    }
}

SKSEPluginLoad(const SKSE::LoadInterface *skse) {

    SetupLog();
    logger::info("Plugin loaded");
    SKSE::Init(skse);
    GetVanillaPointerFromStealer();
    SKSE::GetMessagingInterface()->RegisterListener(OnMessage);
    return true;
}
