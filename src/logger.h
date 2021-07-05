//
// Created by zhong on 7/4/21.
//

#ifndef ARVCC_SRC_LOGGER_H_
#define ARVCC_SRC_LOGGER_H_

#include <memory>

class ExprAST;
class PrototypeAST;

std::unique_ptr<ExprAST> LogError(const char *str);

std::unique_ptr<PrototypeAST> LogErrorP(const char *str);



#endif //ARVCC_SRC_LOGGER_H_
