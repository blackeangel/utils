// tar_repacker.cpp — UtilBase wrapper for tar_repacker
// All logic lives in src/tar_repacker/{archive_rw,metadata,extract,pack}.cpp
// This file implements only the UtilBase interface.

#include "../include/main.hpp"

// Forward declarations from src/tar_repacker/
void extract_tar(const std::string& tar_path, const std::filesystem::path& out_dir);
void pack_tar(const std::filesystem::path& src_dir, const std::string& tar_path);

// ── Derive default output dir: "system.tar.gz" → "system" ─────────────────────
static std::string tar_default_outdir(const std::string& archive) {
    std::filesystem::path p(archive);
    std::string stem = p.stem().string();
    if (stem.size() > 4 && stem.substr(stem.size() - 4) == ".tar")
        stem = stem.substr(0, stem.size() - 4);
    return stem;
}

// ── UtilBase implementation ────────────────────────────────────────────────────

void TarRepacker::show_help() {
    std::cout <<
        "\ntar_repacker\n\n"
        "Android TAR repacker with PAX / SELinux / capabilities support.\n\n"
        "Usage:\n\n"
        "  tar_repacker extract <archive.tar[.gz|.bz2|.xz|.zst]> [output_dir]\n"
        "  tar_repacker pack    <source_dir> <output.tar>\n\n"
        "  extract\n"
        "    Unpacks all entries from the archive into <output_dir>.\n"
        "    <output_dir> defaults to the archive name without extension(s).\n"
        "    Writes metadata (uid, gid, mode, SELinux context, capabilities,\n"
        "    symlink target) to <output_dir>/_metadata.txt.\n"
        "    Symlinks are extracted as empty regular files (safe on all OS).\n\n"
        "  pack\n"
        "    Reads <source_dir>/_metadata.txt to restore uid/gid/mode/SELinux/\n"
        "    capabilities and rebuilds the original TAR with PAX extended headers.\n"
        "    Output is always a plain (uncompressed) TAR.\n\n"
        "  Supported input formats: tar, tar.gz, tar.bz2, tar.xz, tar.zst, tar.lz4\n\n"
        "Examples:\n\n"
        "  utils tar_repacker extract system.tar.gz\n"
        "  utils tar_repacker extract system.tar ./work/system\n"
        "  utils tar_repacker pack    ./work/system system_new.tar\n\n";
}

ParseResult TarRepacker::parse_cmd_line(int argc, char* argv[]) {
    // argv[0] = extract|pack, further args follow
    if (argc < 1) {
        show_help();
        return ParseResult::not_enough;
    }

    mode_.clear();
    archive_path_.clear();
    dir_path_.clear();

    std::string_view cmd = argv[0];

    if (cmd == "extract") {
        if (argc < 2) {
            std::cerr << "Error: 'extract' requires an archive path.\n\n";
            show_help();
            return ParseResult::not_enough;
        }
        mode_         = "extract";
        archive_path_ = argv[1];
        dir_path_     = (argc >= 3) ? argv[2] : tar_default_outdir(archive_path_);

    } else if (cmd == "pack") {
        if (argc < 3) {
            std::cerr << "Error: 'pack' requires <source_dir> and <output.tar>.\n\n";
            show_help();
            return ParseResult::not_enough;
        }
        mode_         = "pack";
        dir_path_     = argv[1];
        archive_path_ = argv[2];

    } else {
        std::cerr << "Error: unknown command '" << cmd << "'. Use 'extract' or 'pack'.\n\n";
        show_help();
        return ParseResult::wrong_option;
    }

    return ParseResult::ok;
}

ProcessResult TarRepacker::process() {
    try {
        if (mode_ == "extract") {
            std::cout << "Extracting '" << archive_path_
                      << "' → '" << dir_path_ << "'\n";
            extract_tar(archive_path_, std::filesystem::path(dir_path_));

        } else {
            if (!std::filesystem::is_directory(dir_path_)) {
                std::cerr << "Error: '" << dir_path_
                          << "' is not a directory.\n";
                return ProcessResult::open_error;
            }
            std::cout << "Packing '" << dir_path_
                      << "' → '" << archive_path_ << "'\n";
            pack_tar(std::filesystem::path(dir_path_), archive_path_);
        }
    } catch (const std::exception& ex) {
        std::cerr << "tar_repacker error: " << ex.what() << "\n";
        return ProcessResult::breaked;
    }
    return ProcessResult::ok;
}
