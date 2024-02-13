#pragma once

#include "Utils.hh"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

enum LANGUAGE_ACTION_TYPE { INTERPRET, COMPILE, JIT };

enum EXPRESSION_TYPE {
    INTEGER_EXP,
    BOOLEAN_EXP,
    VARIABLE_EXP,
    UNARY_OP_EXP,
    BINARY_OP_EXP,
    IF_EXP,
    FUNCTION_CALL_EXP,
};

enum TYPE {
    BOOL_T,
    INT_T,
};

inline std::string ToString(TYPE type) {
    switch (type) {
    case BOOL_T:
        return "bool";
    case INT_T:
        return "int";
    }
}

enum UNARY_OPERATOR {
    NOT_OP,
    NEG_OP,
};

enum BINARY_OPERATOR {
    ADD_OP,
    MUL_OP,
    DIV_OP,
    MOD_OP,
    AND_OP,
    OR_OP,
    EQS_OP,
    NEQ_OP,
    GT_OP,
    GTE_OP,
    LT_OP,
    LTE_OP,
};

inline std::string ToString(BINARY_OPERATOR op) {
    switch (op) {
    case ADD_OP:
        return "+";
    case MUL_OP:
        return "*";
    case DIV_OP:
        return "/";
    case MOD_OP:
        return "%";
    case AND_OP:
        return "&&";
    case OR_OP:
        return "||";
    case EQS_OP:
        return "==";
    case NEQ_OP:
        return "!=";
    case GT_OP:
        return ">";
        break;
    case GTE_OP:
        return ">=";
    case LT_OP:
        return "<";
    case LTE_OP:
        return "<=";
    }
}

class ValueDef;
class FunctionDef;

class BaseExpression;
class UnaryOperator;
class BinaryOperator;
class IfOperator;
class Expression;

class ValueDef {
  public:
    TYPE type;
    std::string name;
    std::unique_ptr<Expression> expression;

    bool semantics_verified = false;
    bool semantics_correct = false;

    inline ValueDef(TYPE type, std::string name,
                    std::unique_ptr<Expression> expression)
        : type(type), name(name), expression(std::move(expression)) {}

    inline virtual ~ValueDef() = default;
    ValueDef &operator=(ValueDef &&other) = default;

    inline friend std::ostream &operator<<(std::ostream &os,
                                           ValueDef const &m) {

        return os << "valdef " << m.name << ": " << ToString(m.type) << " = "
                  << m.expression << ";";
    }

    inline static std::unique_ptr<ValueDef>
    from(TYPE type, std::string name, std::unique_ptr<Expression> expression) {
        std::unique_ptr<ValueDef> result(
            new ValueDef(type, name, std::move(expression)));
        return result;
    }

    bool verify_semantics(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>>
            &globals_ast);

    std::variant<bool, int> interpret(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, std::variant<bool, int>> &context);

    llvm::Value *generate_llvm_ir();
};

class FunctionDef {
  public:
    TYPE return_type;
    std::string name;
    std::vector<std::string> args_name;
    std::vector<TYPE> args_type;
    std::unique_ptr<Expression> expression;

    bool semantics_verified = false;
    bool semantics_correct = false;

    inline FunctionDef() {}

    inline virtual ~FunctionDef() = default;
    FunctionDef &operator=(FunctionDef &&other) = default;

    inline friend std::ostream &operator<<(std::ostream &os,
                                           FunctionDef const &m) {
        os << "funcdef " << m.name;
        for (size_t i = 0; i < m.args_name.size(); i++) {
            os << " " << m.args_name[i] << ": " << m.args_type[i];
        }
        os << " -> " << ToString(m.return_type) << " " << m.expression << ";";
        return os;
    }

    inline void add_argument(TYPE arg_type, std::string arg_name) {
        this->args_name.push_back(arg_name);
        this->args_type.push_back(arg_type);
    }

    inline void set_info(std::string name, TYPE return_type,
                         std::unique_ptr<Expression> expression) {
        this->name = name;
        this->return_type = return_type;
        this->expression = std::move(expression);
    }

    inline static std::unique_ptr<FunctionDef> from() {
        std::unique_ptr<FunctionDef> result(new FunctionDef());
        return result;
    }

    bool verify_semantics(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>>
            &globals_ast);

    std::variant<bool, int> interpret(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, std::variant<bool, int>> &context);

    llvm::Value *generate_llvm_ir();
};

class BaseExpression {
  public:
    TYPE result_type; // type of the computed result
    bool semantics_verified = false;
    bool semantics_correct = false;

    inline virtual ~BaseExpression() = default;
    BaseExpression &operator=(BaseExpression &&other) = default;

    virtual bool verify_semantics(
        TYPE result_type,
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, TYPE> &context) = 0;

    virtual std::variant<bool, int> interpret(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, std::variant<bool, int>> &context) = 0;

    virtual llvm::Value *generate_llvm_ir() = 0;
    virtual TYPE deduce_result_type() = 0;
};

class UnaryOperator : public BaseExpression {
  public:
    std::unique_ptr<Expression> fst;
    UNARY_OPERATOR op_type;

    inline UnaryOperator(std::unique_ptr<Expression> fst,
                         UNARY_OPERATOR op_type)
        : fst(std::move(fst)), op_type(op_type) {}

    inline virtual ~UnaryOperator() = default;
    UnaryOperator &operator=(UnaryOperator &&other) = default;

    inline friend std::ostream &operator<<(std::ostream &os,
                                           UnaryOperator const &m) {
        return os << (m.op_type == NOT_OP ? "!(" : "-(") << m.fst << ")";
    }

    virtual bool verify_semantics(
        TYPE result_type,
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, TYPE> &context) override;
    virtual std::variant<bool, int> interpret(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, std::variant<bool, int>> &context)
        override;
    virtual llvm::Value *generate_llvm_ir() override;
    virtual TYPE deduce_result_type() override;
};

class BinaryOperator : public BaseExpression {
  public:
    std::unique_ptr<Expression> fst;
    std::unique_ptr<Expression> snd;
    BINARY_OPERATOR op_type;

    inline BinaryOperator(std::unique_ptr<Expression> fst,
                          std::unique_ptr<Expression> snd,
                          BINARY_OPERATOR op_type)
        : fst(std::move(fst)), snd(std::move(snd)), op_type(op_type) {}

    inline virtual ~BinaryOperator() = default;
    BinaryOperator &operator=(BinaryOperator &&other) = default;

    inline friend std::ostream &operator<<(std::ostream &os,
                                           BinaryOperator const &m) {

        return os << "(" << m.fst << ") " << ToString(m.op_type) << " ("
                  << m.snd << ")";
    }

    virtual bool verify_semantics(
        TYPE result_type,
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, TYPE> &context) override;
    virtual std::variant<bool, int> interpret(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, std::variant<bool, int>> &context)
        override;
    virtual llvm::Value *generate_llvm_ir() override;
    virtual TYPE deduce_result_type() override;
};

class IfOperator : public BaseExpression {
  public:
    std::unique_ptr<Expression> cond;
    std::unique_ptr<Expression> yes;
    std::unique_ptr<Expression> no;

    inline IfOperator(std::unique_ptr<Expression> cond,
                      std::unique_ptr<Expression> yes,
                      std::unique_ptr<Expression> no)
        : cond(std::move(cond)), yes(std::move(yes)), no(std::move(no)) {}

    inline virtual ~IfOperator() = default;
    IfOperator &operator=(IfOperator &&other) = default;

    inline friend std::ostream &operator<<(std::ostream &os,
                                           IfOperator const &m) {
        return os << "if (" << m.cond << ") then " << m.yes << " else " << m.no;
    }

    virtual bool verify_semantics(
        TYPE result_type,
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, TYPE> &context) override;
    virtual std::variant<bool, int> interpret(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, std::variant<bool, int>> &context)
        override;
    virtual llvm::Value *generate_llvm_ir() override;
    virtual TYPE deduce_result_type() override;
};

class FunctionCall : public BaseExpression {
  public:
    std::string function_name;
    std::vector<std::unique_ptr<Expression>> args;

    inline FunctionCall() {}

    inline FunctionCall(FunctionCall &&fc) {
        this->function_name = fc.function_name;
        this->args = std::move(fc.args);
    }

    inline virtual ~FunctionCall() = default;
    FunctionCall &operator=(FunctionCall &&other) = default;

    inline friend std::ostream &operator<<(std::ostream &os,
                                           FunctionCall const &m) {
        os << m.function_name;
        for (const std::unique_ptr<Expression> &i : m.args) {
            os << " (" << i << ")";
        }
        return os;
    }

    inline void set_function_name(std::string name) {
        this->function_name = name;
    }

    inline void add_argument(std::unique_ptr<Expression> arg) {
        this->args.push_back(std::move(arg));
    }

    virtual bool verify_semantics(
        TYPE result_type,
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, TYPE> &context) override;
    virtual std::variant<bool, int> interpret(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, std::variant<bool, int>> &context)
        override;
    virtual llvm::Value *generate_llvm_ir() override;
    virtual TYPE deduce_result_type() override;
};

class Expression : public BaseExpression {
  public:
    std::variant<int, bool, std::string, std::unique_ptr<UnaryOperator>,
                 std::unique_ptr<BinaryOperator>, std::unique_ptr<IfOperator>,
                 std::unique_ptr<FunctionCall>>
        value;
    EXPRESSION_TYPE type;

    inline Expression(int value) : value(value), type(INTEGER_EXP) {}
    inline Expression(bool value) : value(value), type(BOOLEAN_EXP) {}
    inline Expression(std::string value) : value(value), type(VARIABLE_EXP) {}
    inline Expression(std::unique_ptr<UnaryOperator> value)
        : value(std::move(value)), type(UNARY_OP_EXP) {}
    inline Expression(std::unique_ptr<BinaryOperator> value)
        : value(std::move(value)), type(BINARY_OP_EXP) {}
    inline Expression(std::unique_ptr<IfOperator> value)
        : value(std::move(value)), type(IF_EXP) {}
    inline Expression(FunctionCall value) {
        this->type = FUNCTION_CALL_EXP;
        std::unique_ptr<FunctionCall> f(new FunctionCall(std::move(value)));
        this->value = std::move(f);
    }

    inline virtual ~Expression() = default;
    Expression &operator=(Expression &&other) = default;

    inline friend std::ostream &operator<<(std::ostream &os,
                                           Expression const &m) {
        switch (m.type) {
        case INTEGER_EXP:
            return os << std::get<int>(m.value);
            break;
        case BOOLEAN_EXP:
            return os << std::get<bool>(m.value);
            break;
        case VARIABLE_EXP:
            return os << std::get<std::string>(m.value);
            break;
        case UNARY_OP_EXP:
            return os << std::get<std::unique_ptr<UnaryOperator>>(m.value);
            break;
        case BINARY_OP_EXP:
            return os << std::get<std::unique_ptr<BinaryOperator>>(m.value);
            break;
        case IF_EXP:
            return os << std::get<std::unique_ptr<IfOperator>>(m.value);
            break;
        case FUNCTION_CALL_EXP:
            return os << std::get<std::unique_ptr<FunctionCall>>(m.value);
            break;
        }
    }

    inline static std::unique_ptr<Expression> from(int value) {
        std::unique_ptr<Expression> result(new Expression(value));
        return result;
    }

    inline static std::unique_ptr<Expression> from(bool value) {
        std::unique_ptr<Expression> result(new Expression(value));
        return result;
    }

    inline static std::unique_ptr<Expression> from(std::string value) {
        std::unique_ptr<Expression> result(new Expression(value));
        return result;
    }

    inline static std::unique_ptr<Expression> from(FunctionCall fc) {
        std::unique_ptr<Expression> result(new Expression(std::move(fc)));
        return result;
    }

    inline static std::unique_ptr<Expression>
    from(std::unique_ptr<Expression> fst, UNARY_OPERATOR type) {
        std::unique_ptr<UnaryOperator> unary_op(
            new UnaryOperator(std::move(fst), type));
        std::unique_ptr<Expression> result(new Expression(std::move(unary_op)));
        return result;
    }

    inline static std::unique_ptr<Expression>
    from(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs,
         BINARY_OPERATOR type) {
        std::unique_ptr<BinaryOperator> binary_op(
            new BinaryOperator(std::move(lhs), std::move(rhs), type));
        std::unique_ptr<Expression> result(
            new Expression(std::move(binary_op)));
        return result;
    }

    inline static std::unique_ptr<Expression>
    from(std::unique_ptr<Expression> cond, std::unique_ptr<Expression> yes,
         std::unique_ptr<Expression> no) {
        std::unique_ptr<IfOperator> if_op(
            new IfOperator(std::move(cond), std::move(yes), std::move(no)));
        std::unique_ptr<Expression> result(new Expression(std::move(if_op)));
        return result;
    }

    virtual bool verify_semantics(
        TYPE result_type,
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, TYPE> &context) override;
    virtual std::variant<bool, int> interpret(
        std::unordered_map<std::string, std::unique_ptr<FunctionDef>>
            &functions_ast,
        std::unordered_map<std::string, std::unique_ptr<ValueDef>> &globals_ast,
        std::unordered_map<std::string, std::variant<bool, int>> &context)
        override;
    virtual llvm::Value *generate_llvm_ir() override;
    virtual TYPE deduce_result_type() override;
};

inline std::ostream &
print(std::ostream &os,
      std::unordered_map<std::string, std::unique_ptr<FunctionDef>> &ast) {
    for (auto &i : ast) {
        os << i.second << "\n";
    }
    return os;
}

inline std::ostream &
print(std::ostream &os,
      std::unordered_map<std::string, std::unique_ptr<ValueDef>> &ast) {
    for (auto &i : ast) {
        os << i.second << "\n";
    }
    return os;
}
