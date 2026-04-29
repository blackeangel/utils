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

void Simg2Simg::show_help() {
    fprintf(stderr,
        "\nsimg2simg\n\n"
        "Usage:\n\n"
        "    simg2simg <input_sparse_file> <output_sparse_file_prefix> <max_size>\n\n"
        "    Splits a sparse image into multiple sparse images, each no larger than max_size bytes.\n"
        "    Output files are named <prefix>.0, <prefix>.1, etc.\n\n"
    );
}

ParseResult Simg2Simg::parse_cmd_line(int argc, char* argv[]) {
    // argv+2 already stripped by main: argv[0]=input, argv[1]=prefix, argv[2]=max_size
    if (argc != 3) {
        show_help();
        return ParseResult::not_enough;
    }
    input_path    = argv[0];
    output_prefix = argv[1];
    max_size      = static_cast<int64_t>(std::atoll(argv[2]));
    if (max_size <= 0) {
        fprintf(stderr, "max_size must be a positive integer\n");
        show_help();
        return ParseResult::wrong_option;
    }
    return ParseResult::ok;
}

ProcessResult Simg2Simg::process() {
    int in = open(input_path, O_RDONLY | O_BINARY);
    if (in < 0) {
        fprintf(stderr, "Cannot open input file %s\n", input_path);
        return ProcessResult::open_error;
    }

    struct sparse_file* s = sparse_file_import(in, true, false);
    if (!s) {
        fprintf(stderr, "Failed to import sparse file\n");
        close(in);
        return ProcessResult::read_error;
    }

    int files = sparse_file_resparse(s, static_cast<unsigned int>(max_size), nullptr, 0);
    if (files < 0) {
        fprintf(stderr, "Failed to resparse\n");
        sparse_file_destroy(s);
        close(in);
        return ProcessResult::breaked;
    }

    struct sparse_file** out_s =
        static_cast<struct sparse_file**>(calloc(sizeof(struct sparse_file*), files));
    if (!out_s) {
        fprintf(stderr, "Failed to allocate sparse file array\n");
        sparse_file_destroy(s);
        close(in);
        return ProcessResult::write_error;
    }

    files = sparse_file_resparse(s, static_cast<unsigned int>(max_size), out_s, files);
    if (files < 0) {
        fprintf(stderr, "Failed to resparse\n");
        free(out_s);
        sparse_file_destroy(s);
        close(in);
        return ProcessResult::breaked;
    }

    ProcessResult result = ProcessResult::ok;
    for (int i = 0; i < files; ++i) {
        char filename[4096];
        int len = snprintf(filename, sizeof(filename), "%s.%d", output_prefix, i);
        if (len >= static_cast<int>(sizeof(filename))) {
            fprintf(stderr, "Filename too long\n");
            result = ProcessResult::write_error;
            break;
        }

        int out = open(filename, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0664);
        if (out < 0) {
            fprintf(stderr, "Cannot open output file %s\n", filename);
            result = ProcessResult::open_error;
            break;
        }

        if (sparse_file_write(out_s[i], out, false, true, false)) {
            fprintf(stderr, "Failed to write sparse file %s\n", filename);
            close(out);
            result = ProcessResult::write_error;
            break;
        }
        close(out);
    }

    for (int i = 0; i < files; ++i) {
        sparse_file_destroy(out_s[i]);
    }
    free(out_s);
    sparse_file_destroy(s);
    close(in);
    return result;
}
