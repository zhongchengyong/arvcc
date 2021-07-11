//
// Created by zhong on 6/20/21.
//

#include "token.h"
#include "lexer.h"

#include <cstdio>

std::string identifier_str;
double num_val;
int cur_tok;

int GetNextToken() {
  return cur_tok = GetTok();
}

int GetTok() {
  static int last_char = ' ';
  while (isspace(last_char))
    last_char = getchar();

  if (isalpha(last_char)) {
    identifier_str = last_char;
    while (isalnum(last_char = getchar()))
      identifier_str += last_char;

    if ("def" == identifier_str)
      return tok_def;

    if ("extern" == identifier_str)
      return tok_extern;

    return tok_ident;
  }

  if (isdigit(last_char) || last_char == '.') {
    std::string num_str;
    do {
      num_str += last_char;
      last_char = getchar();
    } while (isdigit(last_char) || last_char == '.');

    num_val = strtod(num_str.c_str(), nullptr);
    return tok_number;
  }

  if (last_char == '#') {
    do {
      last_char = getchar();
    } while (last_char != EOF && last_char != '\n' && last_char != '\r');

    if (last_char == EOF)
      return GetTok();
  }

  // Check for EOF, Do NOT eat the EOF.
  if (last_char == EOF) return tok_eof;

  // otherwise, just return the character as its ascii value.
  int this_char = last_char;
  last_char = getchar();
  return this_char;
}
