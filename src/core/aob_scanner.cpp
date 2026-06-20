#include "aob_scanner.h"
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <fstream>

// ======================================================================
// Parse pattern string "48 8B 0D ?? ?? ?? ?? 48 85 C9" into byte list
// ======================================================================
std::vector<std::pair<uint8_t, bool>> AOBScanner::parse_pattern(const std::string& pattern_hex) {
    std::vector<std::pair<uint8_t, bool>> result;
    std::istringstream stream(pattern_hex);
    std::string token;

    while (stream >> token) {
        if (token == "??" || token == "?") {
            result.emplace_back(0, true); // wildcard
        } else {
            uint8_t byte = (uint8_t)std::stoul(token, nullptr, 16);
            result.emplace_back(byte, false);
        }
    }
    return result;
}

// ======================================================================
// Search for pattern in data buffer
// ======================================================================
bool AOBScanner::search_pattern(const uint8_t* data, size_t data_size,
                                 const std::string& pattern_hex, size_t& match_offset) {
    auto pattern = parse_pattern(pattern_hex);
    if (pattern.empty() || data_size < pattern.size())
        return false;

    for (size_t i = 0; i <= data_size - pattern.size(); ++i) {
        bool matched = true;
        for (size_t j = 0; j < pattern.size(); ++j) {
            if (!pattern[j].second && data[i + j] != pattern[j].first) {
                matched = false;
                break;
            }
        }
        if (matched) {
            match_offset = i;
            return true;
        }
    }
    return false;
}

// ======================================================================
// Find .text section info from PE file on disk
// ======================================================================
bool AOBScanner::find_text_section(const std::string& exe_path, SectionInfo& out) {
    FILE* f = fopen(exe_path.c_str(), "rb");
    if (!f) {
        // Fallback: maybe in Roblox-versions subfolder
        // Try to find via alternate path
        return false;
    }

    // Read DOS header
    IMAGE_DOS_HEADER dos;
    if (fread(&dos, 1, sizeof(dos), f) != sizeof(dos) || dos.e_magic != IMAGE_DOS_SIGNATURE) {
        fclose(f);
        return false;
    }

    // Seek to NT headers
    fseek(f, dos.e_lfanew, SEEK_SET);
    DWORD ntSignature = 0;
    fread(&ntSignature, 1, 4, f);
    if (ntSignature != IMAGE_NT_SIGNATURE) {
        fclose(f);
        return false;
    }

    // Read file header
    IMAGE_FILE_HEADER fileHeader;
    fread(&fileHeader, 1, sizeof(fileHeader), f);

    // Read optional header magic to determine 32/64 bit
    WORD magic = 0;
    fread(&magic, 1, sizeof(magic), f);

    DWORD sectionOffset = 0;
    if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        IMAGE_OPTIONAL_HEADER64 optHeader;
        optHeader.Magic = magic;
        fread((char*)&optHeader + 2, 1, sizeof(optHeader) - 2, f); // skip already-read magic
        sectionOffset = dos.e_lfanew + 4 + sizeof(IMAGE_FILE_HEADER) +
                        sizeof(IMAGE_OPTIONAL_HEADER64);
    }
    else if (magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        IMAGE_OPTIONAL_HEADER32 optHeader;
        optHeader.Magic = magic;
        fread((char*)&optHeader + 2, 1, sizeof(optHeader) - 2, f);
        sectionOffset = dos.e_lfanew + 4 + sizeof(IMAGE_FILE_HEADER) +
                        sizeof(IMAGE_OPTIONAL_HEADER32);
    }
    else {
        fclose(f);
        return false;
    }

    // Read section headers
    fseek(f, sectionOffset, SEEK_SET);
    for (WORD i = 0; i < fileHeader.NumberOfSections; ++i) {
        IMAGE_SECTION_HEADER section;
        if (fread(&section, 1, sizeof(section), f) != sizeof(section))
            break;

        char secName[9] = {};
        memcpy(secName, section.Name, 8);

        if (strcmp(secName, ".text") == 0) {
            out.raw_offset = section.PointerToRawData;
            out.raw_size = section.SizeOfRawData;
            out.virtual_addr = section.VirtualAddress;
            out.virtual_size = section.Misc.VirtualSize;
            fclose(f);
            return true;
        }
    }

    fclose(f);
    return false;
}

// ======================================================================
// Extract RIP-relative address from a matched pattern
// Instruction: 48 8B 0D ?? ?? ?? ??  (7 bytes)
// The 32-bit offset is at bytes 3-6, relative to end of instruction
// ======================================================================
uint64_t AOBScanner::extract_rip_addr(uint64_t module_base, const SectionInfo& text,
                                       size_t match_file_offset, const uint8_t* data) {
    // Offset of RIP-relative value in the instruction (bytes 3-6)
    int32_t rel_offset = *(int32_t*)(data + match_file_offset + 3);

    // File offset within the .text section
    size_t file_offset_in_section = match_file_offset - text.raw_offset;

    // RVA of the instruction
    uint32_t instr_rva = text.virtual_addr + (uint32_t)file_offset_in_section;

    // Absolute address of the instruction in the remote process
    uint64_t instr_addr = module_base + instr_rva;

    // For "mov reg, [rip+offset]" (7 bytes: 48 8B ?? ?? ?? ?? ??)
    // The RIP at execution time points to the NEXT instruction
    // So: target = instr_addr + 7 + rel_offset
    uint64_t target = instr_addr + 7 + rel_offset;

    return target;
}

// ======================================================================
// Scan for FakeDataModel global pointer
// Pattern: mov rcx, [rip+offset]; test rcx, rcx; je short; mov rcx, [rcx+field]
// Bytes:   48 8B 0D ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 49 ??
// ======================================================================
AOBResult AOBScanner::find_fakemodel(uint64_t module_base, const std::string& exe_path) {
    AOBResult result;

    SectionInfo text;
    if (!find_text_section(exe_path, text))
        return result;

    // Read .text section from file
    FILE* f = fopen(exe_path.c_str(), "rb");
    if (!f) return result;

    std::vector<uint8_t> text_data(text.raw_size);
    fseek(f, text.raw_offset, SEEK_SET);
    size_t bytes_read = fread(text_data.data(), 1, text.raw_size, f);
    fclose(f);

    if (bytes_read != text.raw_size)
        return result;

    // Pattern for FakeDataModel access
    const char* pattern = "48 8B 0D ?? ?? ?? ?? 48 85 C9 74 ?? 48 8B 49 ??";

    size_t match_offset = 0;
    if (!search_pattern(text_data.data(), text_data.size(), pattern, match_offset))
        return result;

    uint64_t global_addr = extract_rip_addr(module_base, text, match_offset, text_data.data());
    result.pointer_addr = global_addr;
    result.found = true;

    return result;
}

// ======================================================================
// Scan for VisualEngine global pointer
// Pattern: mov rax, [rip+offset]; test rax, rax; je short; mov rax, [rax+field]
// Bytes:   48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 40 ??
// ======================================================================
AOBResult AOBScanner::find_visualengine(uint64_t module_base, const std::string& exe_path) {
    AOBResult result;

    SectionInfo text;
    if (!find_text_section(exe_path, text))
        return result;

    FILE* f = fopen(exe_path.c_str(), "rb");
    if (!f) return result;

    std::vector<uint8_t> text_data(text.raw_size);
    fseek(f, text.raw_offset, SEEK_SET);
    size_t bytes_read = fread(text_data.data(), 1, text.raw_size, f);
    fclose(f);

    if (bytes_read != text.raw_size)
        return result;

    // VisualEngine access pattern
    const char* pattern = "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 40 ??";
    // Alternative pattern (different register): "48 8B 0D ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 41 ??"
    // Actually, VisualEngine often uses a different pattern. Let me try the most common one.

    size_t match_offset = 0;
    if (!search_pattern(text_data.data(), text_data.size(), pattern, match_offset)) {
        // Try alternative with different offset
        pattern = "48 8B 0D ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 41 ??";
        if (!search_pattern(text_data.data(), text_data.size(), pattern, match_offset))
            return result;
    }

    uint64_t global_addr = extract_rip_addr(module_base, text, match_offset, text_data.data());
    result.pointer_addr = global_addr;
    result.found = true;

    return result;
}

// ======================================================================
// Scan for TaskScheduler global pointer
// Pattern: mov rcx, [rip+offset]; test rax, rax; je short; mov rax, [rax+field]
// Bytes:   48 8B 0D ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 40 ??
// ======================================================================
AOBResult AOBScanner::find_taskscheduler(uint64_t module_base, const std::string& exe_path) {
    AOBResult result;

    SectionInfo text;
    if (!find_text_section(exe_path, text))
        return result;

    FILE* f = fopen(exe_path.c_str(), "rb");
    if (!f) return result;

    std::vector<uint8_t> text_data(text.raw_size);
    fseek(f, text.raw_offset, SEEK_SET);
    size_t bytes_read = fread(text_data.data(), 1, text.raw_size, f);
    fclose(f);

    if (bytes_read != text.raw_size)
        return result;

    // TaskScheduler access pattern
    const char* pattern = "48 8B 0D ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 40 ??";
    // Alternative: sometimes uses 8B 05 (mov eax) instead
    // "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 48 ??"

    size_t match_offset = 0;
    if (!search_pattern(text_data.data(), text_data.size(), pattern, match_offset))
        return result;

    uint64_t global_addr = extract_rip_addr(module_base, text, match_offset, text_data.data());
    result.pointer_addr = global_addr;
    result.found = true;

    return result;
}
