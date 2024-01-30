#pragma once

#include "Utils.hh"
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

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

class FunctionArguments {
  public:
    std::vector<std::string> args_name;
    std::vector<TYPE> args_type;
    size_t length;
};

class BaseExpression {
  public:
    TYPE result_type; // type of the computed result's type
    bool semantics_verified = false;
    bool semantics_correct = false;

    inline virtual ~BaseExpression() = default;

    virtual bool verify_semantics() = 0;

    virtual std::variant<bool, int>
        interpret(/* ???: a context maybe required */) = 0;

    virtual void generate_llvm_ir() = 0;
};

class UnaryOperator;
class BinaryOperator;
class IfOperator;
class Expression;

class UnaryOperator : public BaseExpression {
  public:
    std::unique_ptr<Expression> fst;
    UNARY_OPERATOR op_type;

    inline UnaryOperator(std::unique_ptr<Expression> fst,
                         UNARY_OPERATOR op_type)
        : fst(std::move(fst)), op_type(op_type) {}

    inline virtual ~UnaryOperator() = default;

    inline friend std::ostream &operator<<(std::ostream &os,
                                           UnaryOperator const &m) {
        return os << (m.op_type == NOT_OP ? "!(" : "-(") << m.fst << ")";
    }

    virtual bool verify_semantics() override;
    virtual std::variant<bool, int> interpret() override;
    virtual void generate_llvm_ir() override;
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

    inline friend std::ostream &operator<<(std::ostream &os,
                                           BinaryOperator const &m) {
        std::string op_str;
        switch (m.op_type) {
        case ADD_OP:
            op_str = '+';
            break;
        case MUL_OP:
            op_str = '*';
            break;
        case DIV_OP:
            op_str = '/';
            break;
        case MOD_OP:
            op_str = '%';
            break;
        case AND_OP:
            op_str = "&&";
            break;
        case OR_OP:
            op_str = "||";
            break;
        case EQS_OP:
            op_str = "==";
            break;
        case NEQ_OP:
            op_str = "!=";
            break;
        case GT_OP:
            op_str = '>';
            break;
        case GTE_OP:
            op_str = ">=";
            break;
        case LT_OP:
            op_str = '<';
            break;
        case LTE_OP:
            op_str = "<=";
            break;
        }
        return os << "(" << m.fst << ") " << op_str << " (" << m.snd << ")";
    }

    virtual bool verify_semantics() override;
    virtual std::variant<bool, int> interpret() override;
    virtual void generate_llvm_ir() override;
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

    inline friend std::ostream &operator<<(std::ostream &os,
                                           IfOperator const &m) {
        return os << "if (" << m.cond << ") then " << m.yes << " else " << m.no;
    }

    virtual bool verify_semantics() override;
    virtual std::variant<bool, int> interpret() override;
    virtual void generate_llvm_ir() override;
};

class FunctionCall : public BaseExpression {
  public:
    std::string function_name;
    FunctionArguments args;

    inline virtual ~FunctionCall() = default;

    inline friend std::ostream &operator<<(std::ostream &os,
                                           FunctionCall const &m) {
        // TODO: implement
        return os << m.function_name << "()";
    }

    virtual bool verify_semantics() override;
    virtual std::variant<bool, int> interpret() override;
    virtual void generate_llvm_ir() override;
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
    inline Expression(std::unique_ptr<FunctionCall> value)
        : value(std::move(value)), type(FUNCTION_CALL_EXP) {}

    inline virtual ~Expression() = default;

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

    virtual bool verify_semantics() override;
    virtual std::variant<bool, int> interpret() override;
    virtual void generate_llvm_ir() override;
};