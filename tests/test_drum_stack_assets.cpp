#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static bool read_file(const std::filesystem::path& path, std::string& out) {
    std::ifstream ifs(path);
    if (!ifs) return false;
    out.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return true;
}

static bool contains(const std::string& s, const std::string& needle) {
    return s.find(needle) != std::string::npos;
}

int main() {
    const std::vector<std::filesystem::path> graphs = {
        "../graphs/drum_stack_foundation.json",
        "../graphs/drum_stack_percussion_wash.json",
    };
    for (const auto& g : graphs) {
        std::string json;
        if (!read_file(g, json)) {
            std::cerr << "failed to read graph: " << g << "\n";
            return 1;
        }
        const std::vector<std::string> required = {
            "\"DrumKick\"", "\"DrumSnare\"", "\"DrumHiHat\"",
            "\"DrumClap\"", "\"DrumTom\"", "\"DrumCymbal\""
        };
        for (const auto& tok : required) {
            if (!contains(json, tok)) {
                std::cerr << "graph missing token " << tok << ": " << g << "\n";
                return 1;
            }
        }
    }

    const std::vector<std::filesystem::path> presets = {
        "../factory_presets/drum_kick.json",
        "../factory_presets/drum_snare.json",
        "../factory_presets/drum_hihat.json",
        "../factory_presets/drum_clap.json",
        "../factory_presets/drum_tom.json",
        "../factory_presets/drum_cymbal.json",
    };
    for (const auto& p : presets) {
        std::string json;
        if (!read_file(p, json)) {
            std::cerr << "failed to read preset file: " << p << "\n";
            return 1;
        }
        if (!contains(json, "Stack Foundation")) {
            std::cerr << "preset file missing stack macro preset: " << p << "\n";
            return 1;
        }
    }

    std::cout << "drum stack assets check passed\n";
    return 0;
}
