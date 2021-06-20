//
// Created by zhong on 6/20/21.
//

#ifndef ARVCC_TOKEN_H
#define ARVCC_TOKEN_H

struct Token{
    int token;
    int value;
};

enum {
    T_ADD,
    T_MINUS,
    T_STAR,
    T_SLASH,
    T_INTLIT,  // int literal
};

#endif //ARVCC_TOKEN_H
