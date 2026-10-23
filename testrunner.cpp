
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

// Quote a path so that spaces and special characters are handled.
std::string quotePath(const fs::path& path) {
    std::string s = path.string();

#ifdef _WIN32
    std::string result = "\"";
    for (char c : s) {
        if (c == '"') result += '\\';
        result += c;
    }
    result += '"';
    return result;
#else
    std::string result = "'";
    for (char c : s) {
        if (c == '\'')
            result += "'\\''";
        else
            result += c;
    }
    result += "'";
    return result;
#endif
}

int main(int argc, char* argv[]) {
    // Use the supplied project directory, or the current directory.
    fs::path root = (argc > 1)
        ? fs::absolute(argv[1])
        : fs::current_path();

#ifdef _WIN32
    fs::path program = root / "solution.exe";
#else
    fs::path program = root / "solution";
#endif

    if (!fs::exists(program)) {
        std::cerr << "Error: compiled solution not found:\n"
                  << program << '\n';
        std::cerr << "Compile main.cpp into the solution executable first.\n";
        return 1;
    }

    std::vector<fs::path> inputs;

    // Find all input text files in the project root.
    for (const auto& entry : fs::directory_iterator(root)) {
        if (!entry.is_regular_file())
            continue;

        const std::string name = entry.path().filename().string();

        if (name.rfind("in_", 0) == 0 &&
            entry.path().extension() == ".txt") {
            inputs.push_back(entry.path());
        }
    }

    // Ensure a predictable processing order.
    std::sort(inputs.begin(), inputs.end());

    if (inputs.empty()) {
        std::cout << "No in_*.txt files found in " << root << '\n';
        return 0;
    }

    int passed = 0;
    int failed = 0;

    for (const auto& input : inputs) {
        // Each input gets its own output file.
        fs::path output = root /
            ("output_" + input.stem().string() + ".txt");

        std::string command =
            quotePath(program) +
            " < " + quotePath(input) +
            " > " + quotePath(output);

        std::cout << "Running: " << input.filename().string()
                  << " -> " << output.filename().string() << '\n';

        int result = std::system(command.c_str());

        if (result == 0) {
            ++passed;
            std::cout << "  OK\n";
        } else {
            ++failed;
            std::cerr << "  FAILED (exit status "
                      << result << ")\n";
        }
    }

    std::cout << "\nFinished processing " << inputs.size()
              << " input files.\n"
              << "Successful: " << passed << '\n'
              << "Failed: " << failed << '\n';

    return failed == 0 ? 0 : 1;
}
