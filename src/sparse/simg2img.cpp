/*
 * Copyright (C) 2010 The Android Open Source Project
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

void Simg2Img::show_help() {
    fprintf(stderr,
        "\nsimg2img\n\n"
        "Usage:\n\n"
        "    simg2img <sparse_image_file> [<sparse_image_file2> ...] <raw_image_file>\n\n"
        "    Converts one or more sparse image files to a single raw image.\n\n"
    );
}

ParseResult Simg2Img::parse_cmd_line(int argc, char* argv[]) {
    // argv+2 already stripped by main:
    // argv[0..argc-2] = input sparse files, argv[argc-1] = output raw file
    if (argc < 2) {
        show_help();
        return ParseResult::not_enough;
    }
    output_path = argv[argc - 1];
    for (int i = 0; i < argc - 1; ++i) {
        input_paths.emplace_back(argv[i]);
    }
    return ParseResult::ok;
}

ProcessResult Simg2Img::process() {
    int out = open(output_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0664);
    if (out < 0) {
        fprintf(stderr, "Cannot open output file %s\n", output_path.c_str());
        return ProcessResult::open_error;
    }

    for (const auto& inp : input_paths) {
        int in;
        if (inp == "-") {
            in = STDIN_FILENO;
        } else {
            in = open(inp.c_str(), O_RDONLY | O_BINARY);
            if (in < 0) {
                fprintf(stderr, "Cannot open input file %s\n", inp.c_str());
                close(out);
                return ProcessResult::open_error;
            }
        }

        struct sparse_file* s = sparse_file_import(in, true, false);
        if (!s) {
            fprintf(stderr, "Failed to read sparse file %s\n", inp.c_str());
            if (in != STDIN_FILENO) close(in);
            close(out);
            return ProcessResult::read_error;
        }

        if (lseek(out, 0, SEEK_SET) == -1) {
            perror("lseek failed");
            sparse_file_destroy(s);
            if (in != STDIN_FILENO) close(in);
            close(out);
            return ProcessResult::seek_error;
        }

        if (sparse_file_write(s, out, false, false, false) < 0) {
            fprintf(stderr, "Cannot write output file\n");
            sparse_file_destroy(s);
            if (in != STDIN_FILENO) close(in);
            close(out);
            return ProcessResult::write_error;
        }

        sparse_file_destroy(s);
        if (in != STDIN_FILENO) close(in);
    }

    close(out);
    return ProcessResult::ok;
}
