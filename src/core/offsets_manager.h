#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

#define NOMINMAX
#include <windows.h>

// ------------------------------------------------------------------
// OffsetsManager - Auto-Updater de Offsets via WinHTTP
// ------------------------------------------------------------------
// Fetch desde https://imtheo.lol/Offsets (API RbxDumperV2)
// Fallback a offsets.json local si el servidor falla.
// ------------------------------------------------------------------

struct OffsetsEntry {
    uintptr_t value = 0;       // Valor del offset en decimal
    std::string hex;           // Representación hexadecimal para debug
};

class OffsetsManager {
public:
    static OffsetsManager& instance();

    // Carga los offsets: primero intenta fetch remoto, fallback a local
    bool load();

    // Retorna el valor uintptr_t de un offset por su nombre completo, ej: "Instance/ChildrenStart"
    uintptr_t get_offset(const std::string& group, const std::string& name) const;

    // Retorna la representación hex del offset para debug
    std::string get_hex_offset(const std::string& group, const std::string& name) const;

    // Retorna true si los offsets se cargaron correctamente
    bool is_loaded() const { return loaded_; }

    // Retorna la versión del cliente (del JSON)
    std::string client_version() const { return client_version_; }

private:
    OffsetsManager() = default;
    ~OffsetsManager() = default;
    OffsetsManager(const OffsetsManager&) = delete;
    OffsetsManager& operator=(const OffsetsManager&) = delete;

    // Fetch remoto via WinHTTP
    bool fetch_from_server(std::string& out_json);

    // Carga desde archivo local
    bool load_from_file(const std::string& path, std::string& out_json);

    // Guarda JSON a disco para fallback futuro
    bool save_to_file(const std::string& path, const std::string& json);

    // Parsea el JSON y llena el cache
    bool parse_json(const std::string& json);

    std::unordered_map<std::string, OffsetsEntry> cache_;
    std::string client_version_;
    bool loaded_ = false;

    static constexpr const char* REMOTE_URL = "https://imtheo.lol/Offsets";
    static constexpr const char* LOCAL_FILE = "offsets.json";
};

// Global access macro
#define OFFSET_MGR OffsetsManager::instance()
