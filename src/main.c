#include <assert.h>
#include <datatype99.h>
#include <lang.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct SharedFileHandle {
    FILE *handle;
    int *ref_count;
} SharedFileHandle;

void SharedFileHandleDtor(SharedFileHandle *handle) {
    if (handle && handle->ref_count) {
        (*handle->ref_count)--;
        if (*handle->ref_count == 0) {
            fclose(handle->handle);
            free(handle->ref_count);
        }
    }
}

SharedFileHandle SharedFileHandleShare(SharedFileHandle *handle) {
    if (handle && handle->ref_count) {
        (*handle->ref_count)++;
    }
    return *handle;
}

SharedFileHandle CreateSharedFileHandle(const char *path, const char *flags) {
    SharedFileHandle handle = {.handle = fopen(path, flags), .ref_count = calloc(1, sizeof(int))};
    if (handle.handle && handle.ref_count) {
        *handle.ref_count = 1;
    }
    return handle;
}

#define SharedFCtor(name) __attribute__((cleanup(SharedFileHandleDtor))) SharedFileHandle name

size_t ReadFile(SharedFileHandle handle, char **str) {
    assert(str != NULL);
    assert(*str == NULL);
    assert(handle.handle != NULL);

    fseek(handle.handle, 0, SEEK_END);
    size_t file_size = ftell(handle.handle);
    fseek(handle.handle, 0, SEEK_SET);

    *str = calloc(1, file_size + 1);
    if (!*str) {
        perror("Memory allocation failed");
        return 0;
    }

    size_t read_size = fread(*str, 1, file_size, handle.handle);
    if (read_size != file_size) {
        perror("File read error");
        free(*str);
        *str = NULL;
        return 0;
    }
    return file_size;
}

void PrintToken(Token tok) {
    match(tok) {
        of(String, start, end) printf("[STRING]: %.*s\n", (int)(*end - *start), *start);
        of(Identifier, start, end) printf("[IDENTIFIER]: %.*s\n", (int)(*end - *start), *start);
        of(Number, start, end) printf("[NUMBER]: %.*s\n", (int)(*end - *start), *start);
        of(LParen, start) printf("[LPAREN]\n");
        of(RParen, start) printf("[RPAREN]\n");
        of(LSquare, start) printf("[LSQUARE]\n");
        of(RSquare, start) printf("[RSQUARE]\n");
        of(LAngle, start) printf("[LANGLE]\n");
        of(RAngle, start) printf("[RANGLE]\n");
        of(LCurly, start) printf("[LCURLY]\n");
        of(RCurly, start) printf("[RCURLY]\n");
        of(Coma, start) printf("[COMA]\n");
        of(Semicolon, start) printf("[SEMICOLON]\n");
        of(Unknown) printf("[UNKNOWN]: ERROR, SHOULD NOT HAPPEN\n");
        of(TokEOF) printf("[EOF]\n");
    }
}

bool GetRidOfComments(char *start, char *end) {
    while (start < end) {
        if (*start == '"') {
            // TODO: String parsing to skip them so we don't harm them while getting
            // rid of comments
            start++;
            while (*start != '"') {
                if (start == end || *start == '\n' || *start == '\r') {
                    assert(false && "Unterminated string");
                }
                start++;
            }
        } else if (*start == '/') {
            // TODO: check if this is a single line comment, if yes just fill it with
            // spaces untill we get to the \n else we check if the next char is * then
            // it's a multi line comment and we fill the rest with spaces untill we
            // find the */
        }
        start++;
    }
    return true;
}

int main(int argc, char **argv) {
    char *source = NULL;
    SharedFCtor(file_handle) = CreateSharedFileHandle("test.c", "r");

    if (!file_handle.handle) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    size_t len = ReadFile(SharedFileHandleShare(&file_handle), &source);
    if (len == 0 || !source) {
        fprintf(stderr, "File read failed or empty file.\n");
        return EXIT_FAILURE;
    }

    char *start = source;
    char *end = source + len;
    if (!GetRidOfComments(start, end)) {
        return EXIT_FAILURE;
    }
    while (start < end) {
        Token tok = ParseToken(&start, end);
        PrintToken(tok);
        if (tok.tag == TokEOFTag) {
            break;
        }
    }

    free(source);
    return EXIT_SUCCESS;
}
