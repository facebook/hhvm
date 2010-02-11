
#ifndef __EVAL_BYTECODE_BYTECODE_H__
#define __EVAL_BYTECODE_BYTECODE_H__

#include <cpp/eval/base/eval_base.h>

namespace HPHP {
namespace Eval {
///////////////////////////////////////////////////////////////////////////////

class VariantStack;
class ByteCodeProgram;


enum ArgType {
  NoArg,
  IntArg,
  DblArg,
  StrArg
};

#define OPERATIONS \
  OPERATION(Nop, NoArg) \
  OPERATION(Var, IntArg) \
  OPERATION(VarInd, NoArg) \
  OPERATION(SetVar, IntArg) \
  OPERATION(SetVarInd, NoArg) \
  OPERATION(VarRef, IntArg) \
  OPERATION(VarRefInd, NoArg) \
  OPERATION(Bind, NoArg) \
  OPERATION(BindRef, NoArg) \
  OPERATION(Int, IntArg) \
  OPERATION(String, StrArg) \
  OPERATION(Double, DblArg) \
  OPERATION(Bool, IntArg) \
  OPERATION(Null, NoArg) \
  OPERATION(Echo, NoArg) \
  OPERATION(LogXor, NoArg) \
  OPERATION(BitOr, NoArg) \
  OPERATION(BitAnd, NoArg) \
  OPERATION(BitXor, NoArg) \
  OPERATION(Concat, NoArg) \
  OPERATION(Add, NoArg) \
  OPERATION(Sub, NoArg) \
  OPERATION(Mul, NoArg) \
  OPERATION(Div, NoArg) \
  OPERATION(Mod, NoArg) \
  OPERATION(Sl, NoArg) \
  OPERATION(Sr, NoArg) \
  OPERATION(Same, NoArg) \
  OPERATION(NotSame, NoArg) \
  OPERATION(Equal, NoArg) \
  OPERATION(NotEqual, NoArg) \
  OPERATION(LT, NoArg) \
  OPERATION(LEQ, NoArg) \
  OPERATION(GT, NoArg) \
  OPERATION(GEQ, NoArg) \
  OPERATION(Jmp, IntArg) \
  OPERATION(JmpIf, IntArg) \
  OPERATION(JmpIfNot, IntArg) \
  OPERATION(Discard, NoArg) \

class ByteCode {
public:
  enum Operation {
#define OPERATION(name, arg) name,
    OPERATIONS
#undef OPERATION
  };

  ByteCode() : m_op(Nop) {
    m_arg.ptr = NULL;
  }
  ByteCode(const ByteCode &bc) {
    m_op = bc.m_op;
    m_arg.ptr = bc.m_arg.ptr;
  }
  ByteCode(Operation op, void *arg = NULL) : m_op(op) {
    m_arg.ptr = arg;
  }
  ByteCode(Operation op, int64 arg) : m_op(op) {
    m_arg.num = arg;
  }
  ByteCode(Operation op, double arg) : m_op(op) {
    m_arg.dbl = arg;
  }
  Operation operation() const {
    return m_op;
  }
  void *arg() const {
    return m_arg.ptr;
  }
  int64 intArg() const {
    return m_arg.num;
  }
  double dblArg() const {
    return m_arg.dbl;
  }

  void toString(std::ostringstream &res) const;

private:
  friend class ByteCodeProgram;
  Operation m_op;
  union {
    void *ptr;
    int64 num;
    double dbl;
  } m_arg;
};

class ByteCodeProgram : private std::vector<ByteCode> {
public:
  void add(ByteCode::Operation op, void *arg = NULL);
  void add(ByteCode::Operation op, int64 arg);
  void add(ByteCode::Operation op, int arg);
  void add(ByteCode::Operation op, double arg);
  typedef int64 JumpTag;
  typedef int64 Label;
  JumpTag jump();
  JumpTag jumpIf();
  JumpTag jumpIfNot();
  Label here() const;
  void bindJumpTag(JumpTag t, Label l = - 1);
  void execute(VariantStack &stack, VariableEnvironment &env);
  std::string toString() const;
};


///////////////////////////////////////////////////////////////////////////////
}
}

#endif /* __EVAL_BYTECODE_BYTECODE_H__ */
