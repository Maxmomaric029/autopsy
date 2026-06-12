#include "cache.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <cmath>
#include "log.h"
#include "global.h"

// Thread-safe clock (not ImGui::GetTime — not thread-safe, crashes if context doesn't exist yet)
static double now_sec() {
    using namespace std::chrono;
    static const auto epoch = steady_clock::now();
    return duration<double>(steady_clock::now() - epoch).count();
}
#include "../features/phantom.h"
#include "../features/wallcheck.h"

namespace cache {

    // ==================================================================
    // Part name mapping — both conventions (F1.7)
    // ==================================================================
    struct partmap {
        std::string_view name;
        sdk::instance sdk::player::* Member;
    };

    constexpr partmap Part_Mappings[] = {
        {"Humanoid", &sdk::player::humanoid},
        {"HumanoidRootPart", &sdk::player::HumanoidRootPart},
        {"Head", &sdk::player::Head},
        {"Torso", &sdk::player::Torso},
        {"UpperTorso", &sdk::player::UpperTorso},
        {"LowerTorso", &sdk::player::LowerTorso},
        {"Left Arm", &sdk::player::LeftArm},
        {"LeftArm", &sdk::player::LeftArm},
        {"Right Arm", &sdk::player::RightArm},
        {"RightArm", &sdk::player::RightArm},
        {"Left Leg", &sdk::player::LeftLeg},
        {"LeftLeg", &sdk::player::LeftLeg},
        {"Right Leg", &sdk::player::RightLeg},
        {"RightLeg", &sdk::player::RightLeg},
        {"LeftUpperLeg", &sdk::player::LeftUpperLeg},
        {"RightUpperLeg", &sdk::player::RightUpperLeg},
        {"LeftLowerLeg", &sdk::player::LeftLowerLeg},
        {"RightLowerLeg", &sdk::player::RightLowerLeg},
        {"LeftFoot", &sdk::player::LeftFoot},
        {"RightFoot", &sdk::player::RightFoot},
        {"LeftHand", &sdk::player::LeftHand},
        {"RightHand", &sdk::player::RightHand},
        {"LeftUpperArm", &sdk::player::LeftUpperArm},
        {"RightUpperArm", &sdk::player::RightUpperArm},
        {"LeftLowerArm", &sdk::player::LeftLowerArm},
        {"RightLowerArm", &sdk::player::RightLowerArm}
    };

    std::unordered_map<std::string, sdk::instance sdk::player::*> partlookup() {
        std::unordered_map<std::string, sdk::instance sdk::player::*> Map;
        Map.reserve(sizeof(Part_Mappings) / sizeof(Part_Mappings[0]));
        for (const auto& Mapping : Part_Mappings)
            Map.emplace(Mapping.name, Mapping.Member);
        return Map;
    }

    const auto Part_Lookup = partlookup();
    std::atomic<bool> References_Updated{ false };
    std::atomic<std::uint64_t> Current_GameID{ 0 };
    std::mutex Mutex;

    // ==================================================================
    // Part address cache — keyed by character.Address (F1.7)
    // ==================================================================
    struct CachedParts {
        std::unordered_map<std::string, uint64_t> addresses; // part name -> address
        int64_t rigType = -1; // -1 = unknown, 0 = R6, 1 = R15
        double lastScan = 0.0;
    };
    static std::unordered_map<uint64_t, CachedParts> s_partCache;
    static std::mutex s_cacheMutex;

    // Determine Rig_Type from available parts (F1.7)
    static int detectRigType(const sdk::player& player) {
        if (player.UpperTorso.Address && player.LowerTorso.Address) return 1; // R15
        if (player.Torso.Address) return 0; // R6
        return -1;
    }

    // Check if a fallback is R15-only (only check if player has UpperTorso)
    static bool isR15Part(const std::string& name) {
        return name.find("UpperArm") != std::string::npos ||
               name.find("LowerArm") != std::string::npos ||
               name.find("UpperLeg") != std::string::npos ||
               name.find("LowerLeg") != std::string::npos ||
               name.find("Foot") != std::string::npos ||
               name.find("Hand") != std::string::npos ||
               name == "UpperTorso" ||
               name == "LowerTorso";
    }

    inline float distance(const sdk::vector3& P1, const sdk::vector3& P2) {
        float Dx = P1.x - P2.x;
        float Dy = P1.y - P2.y;
        float Dz = P1.z - P2.z;
        return std::sqrt(Dx * Dx + Dy * Dy + Dz * Dz);
    }

    bool validpos(const sdk::vector3& Pos) {
        return !std::isnan(Pos.x) && !std::isnan(Pos.y) && !std::isnan(Pos.z);
    }

    // ==================================================================
    // rescan — 500ms instead of 100ms (F1.7)
    // ==================================================================
    void rescan() {
        static std::uint64_t Stored_GameID = 0;
        static int post_change_delay = 0;

        while (true) {
            if (post_change_delay > 0) { --post_change_delay; std::this_thread::sleep_for(std::chrono::milliseconds(100)); continue; }
            try {
                auto fakemodel = drive->read<std::uint64_t>(drive->modulebase() + offset::fakemodel::Pointer);
                global::model.Address = drive->read<std::uint64_t>(fakemodel + offset::fakemodel::RealDataModel);

                if (global::model.Address != 0) {
                    std::uint64_t GameID = drive->read<uint64_t>(global::model.Address + offset::datamodel::PlaceId);

                    if (GameID != Stored_GameID) {
                        Stored_GameID = GameID;
                        Current_GameID.store(GameID);
                        global::GameID = GameID;
                        static bool firstInit = true;
                        if (firstInit && GameID == global::rivals::PlaceId) {
                            global::aim::Aimbot_type = 0;
                            global::silent::SpoofMouse = true;
                        }
                        firstInit = false;

                        global::actor.Address = global::model.childclass("Players").Address;
                        auto Lightin = global::model.childclass("Lighting");
                        global::light = sdk::light(Lightin.Address);
                        global::workspace.Address = global::model.childclass("Workspace").Address;
                        global::camera.Address = global::workspace.childclass("Camera").Address;

                        s_partCache.clear(); // invalidate all cached parts on game change

                        References_Updated.store(true);
                        post_change_delay = 50;
                    }
                }
            }
            catch (...) {}

            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // was 100 (F1.7)
        }
    }

    bool is_dead(const sdk::player& player) {
        if (!player.character.Address) return true;
        if (!player.humanoid.Address) return false;
        float health = drive->read<float>(player.humanoid.Address + offset::humanoid::Health);
        return health <= 0.f;
    }

    // ==================================================================
    // data() — populate player parts with caching and Rig_Type detection (F1.7)
    // ==================================================================
    void data(sdk::player& player, const sdk::vector3& Local_Pos, bool Is_Local) {
        if (player.character.Address == 0) return;

        uint64_t charAddr = player.character.Address;
        double now = now_sec();

        // Check cache
        CachedParts* cached = nullptr;
        {
            std::lock_guard<std::mutex> lock(s_cacheMutex);
            auto it = s_partCache.find(charAddr);
            if (it != s_partCache.end()) {
                cached = &it->second;
                // Refresh volatile data even if cached
            }
        }

        // --- Part scanning ---
        bool needsFullScan = false;
        if (!cached) {
            needsFullScan = true;
        } else if (now - cached->lastScan > 2.0) {
            // Refresh every 2s
            needsFullScan = true;
        }

        std::vector<sdk::instance> children;
        if (needsFullScan) {
            CachedParts newCache;
            children = player.character.children();

            std::unordered_map<std::string, sdk::instance> nameToChild;
            nameToChild.reserve(children.size());
            for (const auto& child : children) {
                nameToChild[child.name()] = child;
            }

            auto getChildByName = [&](const std::string& name) -> sdk::instance {
                auto it = nameToChild.find(name);
                if (it != nameToChild.end()) return it->second;
                return sdk::instance{};
            };

            auto getChildByClass = [&](const std::string& className) -> sdk::instance {
                for (const auto& child : children) {
                    if (child.kind() == className) return child;
                }
                return sdk::instance{};
            };

            // Map all standard parts via Part_Lookup
            for (const auto& [name, child] : nameToChild) {
                auto it = Part_Lookup.find(name);
                if (it != Part_Lookup.end()) {
                    player.*(it->second) = child;
                    newCache.addresses[name] = child.Address;
                }
            }

            // Fallback local lookups (no extra readvm calls for children lists!)
            if (!newCache.addresses.count("Head") && !player.Head.Address) {
                player.Head = getChildByName("Head");
                if (player.Head.Address) newCache.addresses["Head"] = player.Head.Address;
            }
            if (!newCache.addresses.count("HumanoidRootPart") && !player.HumanoidRootPart.Address) {
                player.HumanoidRootPart = getChildByName("HumanoidRootPart");
                if (player.HumanoidRootPart.Address) newCache.addresses["HumanoidRootPart"] = player.HumanoidRootPart.Address;
            }
            if (!newCache.addresses.count("Humanoid") && !player.humanoid.Address) {
                player.humanoid = getChildByClass("Humanoid");
                if (player.humanoid.Address) newCache.addresses["Humanoid"] = player.humanoid.Address;
            }

            // Determine Rig_Type from found parts
            newCache.rigType = detectRigType(player);

            // Only scan parts relevant to the detected rig type using local lookup
            if (newCache.rigType == 0) {
                // R6: Torso, Left/Right Arm, Left/Right Leg
                auto scanR6 = [&](const char* name, sdk::instance sdk::player::* member, const char* altName) {
                    if (!newCache.addresses.count(name) && !(player.*member).Address) {
                        auto inst = getChildByName(name);
                        if (!inst.Address && altName) inst = getChildByName(altName);
                        if (inst.Address) {
                            player.*member = inst;
                            newCache.addresses[name] = inst.Address;
                        }
                    }
                };
                scanR6("Torso", &sdk::player::Torso, nullptr);
                scanR6("Left Arm", &sdk::player::LeftArm, "LeftArm");
                scanR6("Right Arm", &sdk::player::RightArm, "RightArm");
                scanR6("Left Leg", &sdk::player::LeftLeg, "LeftLeg");
                scanR6("Right Leg", &sdk::player::RightLeg, "RightLeg");
            } else if (newCache.rigType == 1) {
                // R15: UpperTorso, LowerTorso, Upper/Lower Arm/Leg, Hand, Foot
                auto scanR15 = [&](const char* name, sdk::instance sdk::player::* member) {
                    if (!newCache.addresses.count(name) && !(player.*member).Address) {
                        auto inst = getChildByName(name);
                        if (inst.Address) {
                            player.*member = inst;
                            newCache.addresses[name] = inst.Address;
                        }
                    }
                };
                scanR15("UpperTorso", &sdk::player::UpperTorso);
                scanR15("LowerTorso", &sdk::player::LowerTorso);
                scanR15("LeftUpperArm", &sdk::player::LeftUpperArm);
                scanR15("RightUpperArm", &sdk::player::RightUpperArm);
                scanR15("LeftLowerArm", &sdk::player::LeftLowerArm);
                scanR15("RightLowerArm", &sdk::player::RightLowerArm);
                scanR15("LeftUpperLeg", &sdk::player::LeftUpperLeg);
                scanR15("RightUpperLeg", &sdk::player::RightUpperLeg);
                scanR15("LeftLowerLeg", &sdk::player::LeftLowerLeg);
                scanR15("RightLowerLeg", &sdk::player::RightLowerLeg);
                scanR15("LeftFoot", &sdk::player::LeftFoot);
                scanR15("RightFoot", &sdk::player::RightFoot);
                scanR15("LeftHand", &sdk::player::LeftHand);
                scanR15("RightHand", &sdk::player::RightHand);
            }

            newCache.lastScan = now;

            // Store in cache
            {
                std::lock_guard<std::mutex> lock(s_cacheMutex);
                s_partCache[charAddr] = std::move(newCache);
            }
        } else {
            // Use cached addresses to set part pointers (fast path)
            // Only need to set parts that were cached — children() is skipped entirely
            for (auto& [name, addr] : cached->addresses) {
                auto it = Part_Lookup.find(name);
                if (it != Part_Lookup.end()) {
                    sdk::instance inst(addr);
                    player.*(it->second) = inst;
                }
            }
            player.Rig_Type = static_cast<uint8_t>(cached->rigType);
        }

        // ---- Volatile data (updated every tick) ----
        if (player.humanoid.Address) {
            sdk::humanoid humanoid(player.humanoid.Address);
            player.Health = humanoid.health();
            player.MaxHealth = humanoid.maxhealth();
            player.Rig_Type = humanoid.rig();
        }

        // Tool name (update fast path during needsFullScan or every 1.0s otherwise)
        bool checkTool = false;
        static std::unordered_map<uint64_t, double> lastToolUpdate;
        if (needsFullScan) {
            checkTool = true;
        } else {
            auto it = lastToolUpdate.find(charAddr);
            if (it == lastToolUpdate.end() || now - it->second > 1.0) {
                checkTool = true;
                lastToolUpdate[charAddr] = now;
            }
        }

        if (checkTool) {
            player.Tool_Name.clear();
            if (needsFullScan) {
                for (const auto& child : children) {
                    if (child.kind() == "Tool") {
                        player.Tool_Name = child.name();
                        break;
                    }
                }
            } else {
                sdk::instance tool = player.character.childclass("Tool");
                if (tool.Address)
                    player.Tool_Name = tool.name();
            }
        }

        // Distance
        if (!Is_Local && player.Head.Address != 0 && global::camera.Address != 0) {
            sdk::part Head(player.Head.Address);
            sdk::vector3 Head_Pos = Head.partposition();
            sdk::camera camera(global::camera.Address);
            sdk::vector3 Camera_Pos = camera.position();
            if (validpos(Head_Pos) && validpos(Camera_Pos))
                player.Distance = distance(Head_Pos, Camera_Pos);
        } else if (!Is_Local && player.HumanoidRootPart.Address != 0 && global::camera.Address != 0) {
            sdk::part Root(player.HumanoidRootPart.Address);
            sdk::vector3 Root_Pos = Root.partposition();
            sdk::camera camera(global::camera.Address);
            sdk::vector3 Camera_Pos = camera.position();
            if (validpos(Root_Pos) && validpos(Camera_Pos))
                player.Distance = distance(Root_Pos, Camera_Pos);
        }
    }

    void update(const sdk::vector3& Local_Pos, const std::string& Local_Name) {
        if (global::actor.Address == 0)
            return;

        sdk::actor Local_SDK_Player = global::actor.local();
        sdk::instance Local_Inst(Local_SDK_Player.Address);

        auto Player_Instances = global::actor.children();

        std::vector<sdk::player> actor;
        actor.reserve(Player_Instances.size());

        for (const auto& instance : Player_Instances) {
            sdk::player player{};
            player.player = sdk::actor(instance.Address);
            player.character = player.player.character();
            player.name = instance.name();
            player.UserID = player.player.userid();
            player.Display_Name = player.player.display();
            player.Local_Player = false;

            data(player, Local_Pos, false);

            sdk::actor Entity(instance.Address);
            sdk::actor LocalPlayer(Local_Inst.Address);

            if (global::setting::Team_Check) {
                if (Entity.teamid() == LocalPlayer.teamid())
                    continue;
            }

            // Skip dead players
            if (is_dead(player)) continue;

            // Skip local player
            if (global::LocalPlayer.character.Address && player.character.Address == global::LocalPlayer.character.Address)
                continue;

            actor.push_back(std::move(player));
        }

        {
            std::lock_guard<std::mutex> Lock(Mutex);
            actor.shrink_to_fit();
            global::Player_Cache = std::move(actor);
        }
        console::playerCount = (int)global::Player_Cache.size();
    }

    void runtime() {
        if (global::actor.Address == 0)
            return;

        sdk::actor LocalPlayerInstance = global::actor.local();
        sdk::instance LocalPlayer2(LocalPlayerInstance.Address);
        if (LocalPlayer2.Address == 0)
            return;

        sdk::instance Player_Instance(LocalPlayer2);

        sdk::player LocalPlayer{};
        LocalPlayer.player = sdk::actor(Player_Instance.Address);
        LocalPlayer.character = Player_Instance.character();
        LocalPlayer.name = Player_Instance.name();
        LocalPlayer.Local_Player = true;

        sdk::vector3 Local_Position{};
        data(LocalPlayer, Local_Position, true);

        if (LocalPlayer.HumanoidRootPart.Address != 0) {
            sdk::part Local_HumanoidRootPart(LocalPlayer.HumanoidRootPart.Address);
            Local_Position = Local_HumanoidRootPart.partposition();
        }

        global::LocalPlayer = LocalPlayer;
        global::GameID = Current_GameID.load();

        // Update console status
        console::cameraAddress = global::camera.Address;
        console::connected = (global::camera.Address != 0);
        console::playerCount = (int)global::Player_Cache.size();
        if (global::GameID == 292439477) {
            std::vector<sdk::player> players;
            cacheplayer(players, Local_Position, LocalPlayer.name);
            rescancache(players, Local_Position, LocalPlayer.name);
            {
                std::lock_guard<std::mutex> lock(Mutex);
                global::Player_Cache = std::move(players);
            }
        } else {
            update(Local_Position, LocalPlayer.name);
        }
    }
}

void cache::run() {
    std::thread(rescan).detach();
    int wallcheck_counter = 0;

    while (true) {
        try {
            if (References_Updated.exchange(false)) {
                std::lock_guard<std::mutex> lock(Mutex);
                global::Player_Cache.clear();
                wallcheck::update_cache();
            }

            if (++wallcheck_counter >= 7) {
                wallcheck_counter = 0;
                wallcheck::update_cache();
            }

            // Early-out sleep if no valid actor (F1.7)
            if (global::actor.Address == 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            }

            runtime();
        }
        catch (...) {}

        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}
