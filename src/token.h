//
// Created by zhong on 6/20/21.
//

#ifndef ARVCC_TOKEN_H
#define ARVCC_TOKEN_H

#include <string>

enum Token {
  tok_eof = -1,

  tok_def = -2,
  tok_extern = -3,
  tok_ident = -4,
  tok_number = -5,
};

#endif //ARVCC_TOKEN_H
