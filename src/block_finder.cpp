#include "../include/main.hpp"

struct BlockInfo {
    std::string name;
    std::string path;
    uint64_t size{};

    // Comparison operator for sorting by name
    bool operator<(const BlockInfo& other) const {
        return name < other.name;
    }
};

void Block_Finder::show_help() {
    std::cout <<
              R"***(
block_finder

Usage:

  block_finder <output_path_file>
    <output_path_file> - optionaly - path to output file with informations
                         of blocks. Default: /sdcard/block_info.txt
)***";
}



// Функция для извлечения текстовой части имени устройства
std::string extractBaseName(const std::string& deviceName) {
    size_t pos = 0;
    while (pos < deviceName.size() && !std::isdigit(deviceName[pos])) {
        ++pos;
    }
    if (pos > 0 && deviceName[pos - 1] == '-') {
        --pos;
    }
    return deviceName.substr(0, pos);
}

// Universal function to find the first matching directory
std::filesystem::path findFirstMatchingDirectory(const std::filesystem::path& rootDir, const std::string& targetName) {
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(rootDir)) {
            if (entry.is_directory() && entry.path().filename() == targetName) {
                return entry.path();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error searching for directory " << targetName << ": " << e.what() << '\n';
    }
    throw std::runtime_error(targetName + " directory not found");
}

// Function to read the size of the block in bytes
uint64_t readBlockSize(const std::string& blockName) {
    try {
        std::filesystem::path sizePath = std::filesystem::path("/sys/class/block") / blockName / "size";
        std::ifstream sizeFile(sizePath);
        if (!sizeFile.is_open()) {
            throw std::ios_base::failure("Failed to open size file");
        }
        uint64_t blockSizeInSectors;
        sizeFile >> blockSizeInSectors;
        return blockSizeInSectors * 512; // Convert sectors to bytes (1 sector = 512 bytes)
    } catch (const std::exception& e) {
        std::cerr << "Error reading block size for " << blockName << ": " << e.what() << '\n';
        return 0;
    }
}

// Function to read the name of the block
std::string readBlockName(const std::string& blockName) {
    try {
        std::string partname = extractBaseName(blockName);
        std::filesystem::path namePath = std::filesystem::path("/sys/class/block") / blockName / partname / "name";
        std::ifstream nameFile(namePath);
        if (!nameFile.is_open()) {
            throw std::ios_base::failure("Failed to open name file");
        }
        std::string blockNameFromFile;
        nameFile >> blockNameFromFile;
        return blockNameFromFile;
    } catch (const std::exception& e) {
        std::cerr << "Error reading block name for " << blockName << ": " << e.what() << '\n';
        return "";
    }
}

std::vector<BlockInfo> getBlockInfos(const std::filesystem::path& devBlockByNameDir) {
    std::vector<BlockInfo> blockInfos;
    std::vector<BlockInfo> superSubBlocks;

    try {
        for (const auto& entry : std::filesystem::directory_iterator(devBlockByNameDir)) {
            if (entry.path().filename().string() == "userdata") {
                continue;
            }
            if (std::filesystem::is_symlink(entry)) {
                try {
                    if (!std::filesystem::exists(entry)) {
                        continue; // Skip if the target does not exist
                    }

                    BlockInfo blockInfo;
                    blockInfo.name = entry.path().filename().string();
                    blockInfo.path = std::filesystem::canonical(entry).string();

                    std::string blockName = std::filesystem::path(blockInfo.path).filename().string();
                    blockInfo.size = readBlockSize(blockName);

                    blockInfos.push_back(blockInfo);

                    if (blockInfo.name == "super") {
                        std::filesystem::path holdersPath = std::filesystem::path("/sys/class/block") / blockName / "holders";
                        for (const auto& file : std::filesystem::directory_iterator(holdersPath)) {
                            std::string fileName = std::filesystem::path(file.path()).filename().string();

                            BlockInfo subBlockInfo;
                            subBlockInfo.name = readBlockName(fileName) + "(" + blockInfo.name + ")";
                            subBlockInfo.path = "/dev/block/" + fileName;
                            subBlockInfo.size = readBlockSize(fileName);

                            superSubBlocks.push_back(subBlockInfo);
                        }
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error processing entry " << entry.path() << ": " << e.what() << '\n';
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error processing directory " << devBlockByNameDir << ": " << e.what() << '\n';
    }

    std::sort(blockInfos.begin(), blockInfos.end());
    std::sort(superSubBlocks.begin(), superSubBlocks.end());
    blockInfos.insert(blockInfos.end(), superSubBlocks.begin(), superSubBlocks.end());

    return blockInfos;
}

ProcessResult Block_Finder::process() {
     try{
         std::filesystem::path devBlockByNameDir = findFirstMatchingDirectory("/dev/block", "by-name");

        // Get block information
         std::vector<BlockInfo> blockInfos = getBlockInfos(devBlockByNameDir);

        // Write the block information to the output file
        std::ofstream outputFile(outputFilename);
        if (!outputFile.is_open()) {
            throw std::ios_base::failure("Failed to open output file");
        }

        for (const auto& block : blockInfos) {
            outputFile << block.name << " = " << block.path << " = " << block.size << "\n";
        }

        std::cout << "Block information has been written to " << outputFilename << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return  ProcessResult::breaked;
    }

    return ProcessResult::ok;
}

ParseResult Block_Finder::parse_cmd_line(int argc, char *argv[]) {
    std::string outputFilename = argc > 1 ? argv[1] : "/sdcard/block_info.txt";
    return ParseResult::ok;
}