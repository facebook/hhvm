
#include <cpp/eval/bytecode/bytecode.h>
#include <cpp/eval/runtime/variant_stack.h>
#include <cpp/base/base_includes.h>
#include <cpp/eval/runtime/variable_environment.h>

namespace HPHP {
namespace Eval {
using namespace std;
///////////////////////////////////////////////////////////////////////////////

static Variant stringArg(const ByteCode &bc) {
  return (StringData*)bc.arg();
}

void ByteCode::toString(ostringstream &res) const {
#define OPERATION(name, argtype)                                        \
  case name:                                                            \
    {                                                                   \
      res << #name;                                                     \
      if (argtype == IntArg) res << " " << intArg();                    \
      else if (argtype == DblArg) res << " " << dblArg();               \
      else if (argtype == StrArg) res << " " << ((StringData*)arg())->data(); \
    }                                                                   \
    break;
  switch (m_op) {
    OPERATIONS
  }
#undef OPERATION
}

void ByteCodeProgram::execute(VariantStack &stack, VariableEnvironment &env) {
#define PUSH stack.push
#define PUSHTMP stack.pushSwap
#define POP stack.topPop
  for (uint pc = 0; pc < size(); ++pc) {
    const ByteCode &bc = operator[](pc);
    switch (bc.operation()) {
    case ByteCode::Nop: break;
    case ByteCode::Var: PUSH(env.getIdx(bc.intArg())); break;
    case ByteCode::VarInd:
      {
        Variant &val = env.get(stack.top());
        stack.pop();
        PUSH(val);
      }
      break;
    case ByteCode::SetVar:
      {
        Variant val(POP());
        env.getIdx(bc.intArg()) = val;
        PUSHTMP(val);
      }
      break;
    case ByteCode::SetVarInd:
      {
        Variant &r = env.get(stack.top(0)) = stack.top(1);
        stack.pop(); stack.pop();
        PUSH(r);
      }
      break;
    case ByteCode::Int: PUSH(bc.intArg()); break;
    case ByteCode::String:
      {
        Variant s(stringArg(bc));
        PUSHTMP(s);
      }
      break;
    case ByteCode::Double: PUSH(bc.dblArg()); break;
    case ByteCode::Bool: PUSH((bool)bc.intArg()); break;
    case ByteCode::Null: PUSH(null_variant); break;
    case ByteCode::Echo: echo(POP()); break;

    case ByteCode::LogXor:
    case ByteCode::BitOr:
    case ByteCode::BitAnd:
    case ByteCode::BitXor:
    case ByteCode::Concat:
    case ByteCode::Add:
    case ByteCode::Sub:
    case ByteCode::Mul:
    case ByteCode::Div:
    case ByteCode::Mod:
    case ByteCode::Sl:
    case ByteCode::Sr:
    case ByteCode::Same:
    case ByteCode::NotSame:
    case ByteCode::Equal:
    case ByteCode::NotEqual:
    case ByteCode::LT:
    case ByteCode::LEQ:
    case ByteCode::GT:
    case ByteCode::GEQ:
      {
        Variant &v2 = stack.top(0);
        Variant &v1 = stack.top(1);
        Variant r;
        switch (bc.operation()) {
        case ByteCode::LogXor:   r = logical_xor(v1, v2); break;
        case ByteCode::BitOr:    r = bitwise_or(v1, v2); break;
        case ByteCode::BitAnd:   r = bitwise_and(v1, v2); break;
        case ByteCode::BitXor:   r = bitwise_xor(v1, v2); break;
        case ByteCode::Concat:   r = concat(v1, v2); break;
        case ByteCode::Add:      r = v1 + v2; break;
        case ByteCode::Sub:      r = v1 - v2; break;
        case ByteCode::Mul:      r = multiply(v1, v2); break;
        case ByteCode::Div:      r = divide(v1, v2); break;
        case ByteCode::Mod:      r = modulo(v1, v2); break;
        case ByteCode::Sl:       r = v1.toInt64() << v2.toInt64(); break;
        case ByteCode::Sr:       r = v1.toInt64() >> v2.toInt64(); break;
        case ByteCode::Same:     r = same(v1, v2); break;
        case ByteCode::NotSame:  r = !same(v1, v2); break;
        case ByteCode::Equal:    r = equal(v1, v2); break;
        case ByteCode::NotEqual: r = !equal(v1, v2); break;
        case ByteCode::LT:       r = less(v1, v2); break;
        case ByteCode::LEQ:      r = not_more(v1, v2); break;
        case ByteCode::GT:       r = more(v1, v2); break;
        case ByteCode::GEQ:      r = not_less(v1, v2); break;
        default:
          ASSERT(false);
        }
        stack.pop();
        stack.pop();
        PUSHTMP(r);
      }
      break;
    case ByteCode::Jmp: pc = bc.intArg() - 1; break;
    case ByteCode::JmpIf:
      if (POP()) {
        pc = bc.intArg() - 1;
      }
      break;
    case ByteCode::JmpIfNot:
      if (!POP()) {
        pc = bc.intArg() - 1;
      }
      break;
    case ByteCode::Discard: stack.pop(); break;
    default:
      throw FatalErrorException("Unsupported bytecode %d", bc.operation());
    }
  }
}

void ByteCodeProgram::add(ByteCode::Operation op, void *arg /* = NULL */) {
  ByteCode bc(op, (void*)arg);
  push_back(bc);
}
void ByteCodeProgram::add(ByteCode::Operation op, int64 arg) {
  ByteCode bc(op, arg);
  push_back(bc);
}
void ByteCodeProgram::add(ByteCode::Operation op, int arg) {
  add(op, (int64)arg);
}
void ByteCodeProgram::add(ByteCode::Operation op, double arg) {
  ByteCode bc(op, arg);
  push_back(bc);
}

ByteCodeProgram::JumpTag ByteCodeProgram::jump() {
  add(ByteCode::Jmp);
  return size() - 1;
}
ByteCodeProgram::JumpTag ByteCodeProgram::jumpIf() {
  add(ByteCode::JmpIf);
  return size() - 1;
}
ByteCodeProgram::JumpTag ByteCodeProgram::jumpIfNot() {
  add(ByteCode::JmpIfNot);
  return size() - 1;
}

ByteCodeProgram::Label ByteCodeProgram::here() const {
  return size();
}
void ByteCodeProgram::bindJumpTag(JumpTag t, Label l /* = -1 */) {
  if (l == -1) l = here();
  operator[](t).m_arg.num = l;
}

string ByteCodeProgram::toString() const {
  ostringstream res;
  int pos = 0;
  for (vector<ByteCode>::const_iterator it = begin(); it != end();
       ++it, ++pos) {
    res << pos << " ";
    it->toString(res);
    res << endl;
  }
  return res.str();
}

///////////////////////////////////////////////////////////////////////////////
}
}

