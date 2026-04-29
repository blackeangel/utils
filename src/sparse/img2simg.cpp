/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _FILE_OFFSET_BITS 64
#define _LARGEFILE64_SOURCE 1

#include "../../include/main.hpp"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sparse/sparse.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define lseek64 lseek
#define off64_t off_t
#endif

void Img2Simg::show_help() {
    fprintf(stderr,
        "\nimg2simg\n\n"
        "Usage:\n\n"
        "    img2simg [-s] <raw_image_file> <sparse_image_file> [<block_size>]\n\n"
        "    -s            Use hole mode (detect sparse regions)\n"
        "    block_size    Block size in bytes (default: 4096, must be multiple of 4)\n\n"
    );
}

ParseResult Img2Simg::parse_cmd_line(int argc, char* argv[]) {
    // argv+2 already stripped by main: argv[0] is the first real arg
    use_hole_mode = false;
    block_size    = 4096;
    input_path    = nullptr;
    output_path   = nullptr;

    int i = 0;
    while (i < argc && argv[i][0] == '-') {
        if (std::string_view(argv[i]) == "-s") {
            use_hole_mode = true;
        } else {
            show_help();
            return ParseResult::wrong_option;
        }
        ++i;
    }

    int remaining = argc - i;
    if (remaining < 2 || remaining > 3) {
        show_help();
        return ParseResult::not_enough;
    }

    input_path  = argv[i];
    output_path = argv[i + 1];
    if (remaining == 3) {
        block_size = static_cast<unsigned int>(std::atoi(argv[i + 2]));
        if (block_size < 1024 || block_size % 4 != 0) {
            fprintf(stderr, "Invalid block size: %u\n", block_size);
            show_help();
            return ParseResult::wrong_option;
        }
    }
    return ParseResult::ok;
}

ProcessResult Img2Simg::process() {
    int in;
    if (std::string_view(input_path) == "-") {
        in = STDIN_FILENO;
    } else {
        in = open(input_path, O_RDONLY | O_BINARY);
        if (in < 0) {
            fprintf(stderr, "Cannot open input file %s\n", input_path);
            return ProcessResult::open_error;
        }
    }

    int out;
    if (std::string_view(output_path) == "-") {
        out = STDOUT_FILENO;
    } else {
        out = open(output_path, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0664);
        if (out < 0) {
            fprintf(stderr, "Cannot open output file %s\n", output_path);
            if (in != STDIN_FILENO) close(in);
            return ProcessResult::open_error;
        }
    }

    off64_t len = lseek64(in, 0, SEEK_END);
    lseek64(in, 0, SEEK_SET);

    struct sparse_file* s = sparse_file_new(block_size, len);
    if (!s) {
        fprintf(stderr, "Failed to create sparse file\n");
        if (in  != STDIN_FILENO)  close(in);
        if (out != STDOUT_FILENO) close(out);
        return ProcessResult::write_error;
    }

    sparse_file_verbose(s);

    enum sparse_read_mode mode = use_hole_mode ? SPARSE_READ_MODE_HOLE : SPARSE_READ_MODE_NORMAL;
    if (sparse_file_read(s, in, mode, false)) {
        fprintf(stderr, "Failed to read file\n");
        sparse_file_destroy(s);
        if (in  != STDIN_FILENO)  close(in);
        if (out != STDOUT_FILENO) close(out);
        return ProcessResult::read_error;
    }

    if (sparse_file_write(s, out, false, true, false)) {
        fprintf(stderr, "Failed to write sparse file\n");
        sparse_file_destroy(s);
        if (in  != STDIN_FILENO)  close(in);
        if (out != STDOUT_FILENO) close(out);
        return ProcessResult::write_error;
    }

    sparse_file_destroy(s);
    if (in  != STDIN_FILENO)  close(in);
    if (out != STDOUT_FILENO) close(out);
    return ProcessResult::ok;
}
