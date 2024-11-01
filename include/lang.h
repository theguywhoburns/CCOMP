#pragma once
#include <assert.h>
#include <ctype.h>

#include <datatype99.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

datatype(
    Token, (String, const char *, const char *), (Identifier, const char *, const char *),
    (Number, const char *, const char *),
    /* We will not support floating point numbers for now */
    (LParen, const char *), (RParen, const char *), (LSquare, const char *),
    (RSquare, const char *), (LAngle, const char *), (RAngle, const char *), (LCurly, const char *),
    (RCurly, const char *), (Coma, const char *), (Semicolon, const char *), (Unknown), (TokEOF));

inline static char *skipspace(char *str) {
    if (!str)
        return NULL;
    while (isspace(*str))
        str++;
    return str;
}

Token ParseToken(char **start, const char *end) {
    assert(start != NULL);
    assert(*start != NULL);
    assert(end != NULL);
    assert(end - *start >= 0);

    *start = skipspace(*start);
    size_t source_len = end - *start;

    if (**start == *end || **start == '\0' || source_len == 0) {
        printf("Reached the end!\n");
        return TokEOF();
    }

    Token ret = Unknown();
    switch (**start) {
    case '"': {
        char *tmp = *start + 1;
        while (*tmp != '"' && *tmp != '\0')
            tmp++;
        if (*tmp == '"') {
            ret = String(*start + 1, tmp);
            *start = tmp + 1;
        } else {
            printf("Unterminated string literal.\n");
            ret = Unknown();
        }
    } break;
    // Fucking love gcc for ...
    case '0' ... '9': {
        char *tmp = *start;
        while (isdigit(*tmp))
            tmp++;
        if (*tmp != '\0' && (isalpha(*tmp))) {
            assert(false && "Cannot start tokens with numbers then continue with letters");
        }
        ret = Number(*start, tmp);
        *start = tmp;
    } break;
    case '_':
    case 'a' ... 'z':
    case 'A' ... 'Z': {
        char *tmp = *start;
        while (isalnum(*tmp) || *tmp == '_')
            tmp++;
        ret = Identifier(*start, tmp);
        *start = tmp;
    } break;

    case ')':
        ret = RParen((*start)++);
        break;
    case '(':
        ret = LParen((*start)++);
        break;
    case ']':
        ret = RSquare((*start)++);
        break;
    case '[':
        ret = LSquare((*start)++);
        break;
    case '>':
        ret = RAngle((*start)++);
        break;
    case '<':
        ret = LAngle((*start)++);
        break;
    case '}':
        ret = RCurly((*start)++);
        break;
    case '{':
        ret = LCurly((*start)++);
        break;
    case ',':
        ret = Coma((*start)++);
        break;
    case ';':
        ret = Semicolon((*start)++);
        break;
    default:
        ret = Unknown();
        break;
    }

    return ret;
}
