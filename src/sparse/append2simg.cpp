/*
 * Copyright (C) 2013 The Android Open Source Project
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
#ifdef _WIN32
#include "asprintf.h"
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sparse/sparse.h"
#include "backed_block.h"
#include "sparse_file.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#if defined(__APPLE__) && defined(__MACH__)
#define lseek64 lseek
#define off64_t off_t
#endif

void Append2Simg::show_help() {
    fprintf(stderr,
        "\nappend2simg\n\n"
        "Usage:\n\n"
        "    append2simg <output> <input>\n\n"
        "    Appends a raw image file to a sparse image file.\n\n"
    );
}

ParseResult Append2Simg::parse_cmd_line(int argc, char* argv[]) {
    // argv+2 already stripped by main: argv[0]=output, argv[1]=input
    if (argc != 2) {
        show_help();
        return ParseResult::not_enough;
    }
    output_path = argv[0];
    input_path  = argv[1];
    return ParseResult::ok;
}

ProcessResult Append2Simg::process() {
    int output = open(output_path, O_RDWR | O_BINARY);
    if (output < 0) {
        fprintf(stderr, "Couldn't open output file (%s)\n", strerror(errno));
        return ProcessResult::open_error;
    }

    struct sparse_file* sparse_output = sparse_file_import_auto(output, false, true);
    if (!sparse_output) {
        fprintf(stderr, "Couldn't import output file\n");
        close(output);
        return ProcessResult::read_error;
    }

    int input = open(input_path, O_RDONLY | O_BINARY);
    if (input < 0) {
        fprintf(stderr, "Couldn't open input file (%s)\n", strerror(errno));
        sparse_file_destroy(sparse_output);
        close(output);
        return ProcessResult::open_error;
    }

    off64_t input_len = lseek64(input, 0, SEEK_END);
    if (input_len < 0) {
        fprintf(stderr, "Couldn't get input file length (%s)\n", strerror(errno));
        close(input); sparse_file_destroy(sparse_output); close(output);
        return ProcessResult::seek_error;
    }
    if (input_len % sparse_output->block_size) {
        fprintf(stderr, "Input file is not a multiple of the output file's block size\n");
        close(input); sparse_file_destroy(sparse_output); close(output);
        return ProcessResult::breaked;
    }
    lseek64(input, 0, SEEK_SET);

    int output_block = sparse_output->len / sparse_output->block_size;
    if (sparse_file_add_fd(sparse_output, input, 0, input_len, output_block) < 0) {
        fprintf(stderr, "Couldn't add input file\n");
        close(input); sparse_file_destroy(sparse_output); close(output);
        return ProcessResult::write_error;
    }
    sparse_output->len += input_len;

    char* tmp_path = nullptr;
    if (asprintf(&tmp_path, "%s.append2simg", output_path) < 0) {
        fprintf(stderr, "Couldn't allocate filename\n");
        close(input); sparse_file_destroy(sparse_output); close(output);
        return ProcessResult::write_error;
    }

    int tmp_fd = open(tmp_path, O_WRONLY | O_CREAT | O_BINARY, 0664);
    if (tmp_fd < 0) {
        fprintf(stderr, "Couldn't open temporary file (%s)\n", strerror(errno));
        free(tmp_path);
        close(input); sparse_file_destroy(sparse_output); close(output);
        return ProcessResult::write_error;
    }

    lseek64(output, 0, SEEK_SET);
    if (sparse_file_write(sparse_output, tmp_fd, false, true, false) < 0) {
        fprintf(stderr, "Failed to write sparse file\n");
        close(tmp_fd); free(tmp_path);
        close(input); sparse_file_destroy(sparse_output); close(output);
        return ProcessResult::write_error;
    }

    sparse_file_destroy(sparse_output);
    close(tmp_fd);
    close(output);
    close(input);

    if (rename(tmp_path, output_path) < 0) {
        fprintf(stderr, "Failed to rename temporary file (%s)\n", strerror(errno));
        free(tmp_path);
        return ProcessResult::write_error;
    }

    free(tmp_path);
    return ProcessResult::ok;
}
