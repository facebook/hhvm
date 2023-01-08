/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2017-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/vm/named-entity-defs.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/vm-regs.h"

#include <algorithm>

namespace HPHP {
namespace VSDEBUG {

namespace {
class FramePointer {
  public:
    FramePointer(Debugger *debugger, int frameDepth)
      : m_fp(nullptr),
      m_exitDummyContext(false) {
        VMRegAnchor _;

        if (debugger->isDummyRequest()) {
          m_fp = vmfp();
          if (m_fp == nullptr && vmStack().count() == 0) {
            g_context->enterDebuggerDummyEnv();
            m_fp = vmfp();
            m_exitDummyContext = true;
          }
        } else {
          m_fp = g_context->getFrameAtDepthForDebuggerUnsafe(frameDepth);
        }
    }

    ~FramePointer() {
      if (m_exitDummyContext) {
        g_context->exitDebuggerDummyEnv();
      }
    }

    operator ActRec *() {
      return m_fp;
    }

    ActRec * operator ->() {
      return m_fp;
    }

  private:
    ActRec *m_fp;
    bool m_exitDummyContext;
};
}

CompletionsCommand::CompletionsCommand(
  Debugger* debugger,
  folly::dynamic message
) : VSCommand(debugger, message),
    m_frameId{0} {

  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  const int frameId = tryGetInt(args, "frameId", -1);
  m_frameId = frameId;
}

CompletionsCommand::~CompletionsCommand() {
}

FrameObject* CompletionsCommand::getFrameObject(DebuggerSession* session) {
  if (m_frameObj != nullptr) {
    return m_frameObj;
  }

  m_frameObj = session->getFrameObject(m_frameId);
  return m_frameObj;
}

request_id_t CompletionsCommand::targetThreadId(DebuggerSession* session) {
  FrameObject* frame = getFrameObject(session);
  if (frame == nullptr) {
    // Execute the completion in the dummy context.
    return Debugger::kDummyTheadId;
  }

  return frame->m_requestId;
}

void CompletionsCommand::addCompletionTarget(
  folly::dynamic& completions,
  const char* completionText,
  const char* completionType,
  int charsToOverwrite
) {
  completions.push_back(folly::dynamic::object);
  folly::dynamic& completion = completions[completions.size() - 1];
  completion["label"] = completionText;
  completion["type"] = completionType;

  // NOTE: "length" in the protocol is the number of chars to overwrite from
  // the input text, counting backwards from "column" position in the
  // CompletionsRequest message. Why this is called "length" is anybody's
  // guess.
  completion["length"] = charsToOverwrite;
}

CompletionsCommand::SuggestionContext
CompletionsCommand::parseCompletionExpression(
  const std::string& expr
) {
  std::string text = expr;
  SuggestionContext context;
  context.type = SuggestionType::None;
  return context;
}

bool CompletionsCommand::executeImpl(
  DebuggerSession* session,
  folly::dynamic* responseMsg
) {
  VMRegAnchor regAnchor;

  // The request thread should not re-enter the debugger while
  // processing this command.
  DebuggerNoBreakContext noBreak(m_debugger);

  folly::dynamic& message = getMessage();
  const folly::dynamic& args = tryGetObject(message, "arguments", s_emptyArgs);
  int column = tryGetInt(args, "column", -1);
  std::string text = tryGetString(args, "text", "");

  if (column == 0) {
    // If no column specified, use the end of the string.
    column = text.size() - 1;
  } else {
    // Completion column is 1-based, but std::string indexes are 0 based.
    column--;
  }

  text = text.substr(0, column);

  folly::dynamic body = folly::dynamic::object;
  body["targets"] = folly::dynamic::array;

  try {
    SuggestionContext context = parseCompletionExpression(text);
    auto& targets = body["targets"];

    // If the target is not paused, completions are only valid for the dummy
    // request thread.
    if (!m_debugger->isPaused() &&
        targetThreadId(session) != Debugger::kDummyTheadId) {
      throw DebuggerCommandException("Target request is running.");
    }

    // Chop off any leading $.
    if (context.matchPrefix[0] == '$') {
      context.matchPrefix = context.matchPrefix.substr(1);
    }

    switch (context.type) {
      case SuggestionType::None:
        break;

      case SuggestionType::Variable:
        addVariableCompletions(context, targets);
        break;

      case SuggestionType::Member:
        addMemberCompletions(context, targets);
        break;

      case SuggestionType::ClassStatic:
        addClassStaticCompletions(context, targets);
        break;

      case SuggestionType::ClassConstant:
        addClassConstantCompletions(context, targets);
        break;

      case SuggestionType::FuncsAndConsts:
        addFuncConstantCompletions(context, targets);
        break;

      default:
        assertx(false);
    }
  } catch (...) {
    // Don't actually report any errors for completion requests, we just
    // return an empty list if something goes wrong.
  }

  (*responseMsg)["body"] = body;

  // Do not resume the target.
  return false;
}

void CompletionsCommand::addVariableCompletions(
  SuggestionContext& context,
  folly::dynamic& targets
) {
  int frameDepth = m_frameObj ? m_frameObj->m_frameDepth : 0;
  auto fp = FramePointer(m_debugger, frameDepth);

  if (fp == nullptr) {
    return;
  }

  // If there is a $this, add it.
  if (
    !fp->isInlined() &&
    fp->func() != nullptr &&
    fp->func()->cls() != nullptr &&
    fp->hasThis()) {
    static const std::string thisName("this");
    if (context.matchPrefix.size() >= thisName.size() &&
        std::equal(
          thisName.begin(),
          thisName.end(),
          context.matchPrefix.begin())) {

      addCompletionTarget(
        targets,
        thisName.c_str(),
        CompletionTypeVar,
        thisName.size()
      );
    }
  }

  // Add any defined variables that match the specified prefix.
  const auto allVariables = getDefinedVariables(fp);

  for (ArrayIter iter(allVariables); iter; ++iter) {
    const std::string& name = iter.first().toString().toCppString();
    addIfMatch(name, context.matchPrefix, CompletionTypeVar, targets);
  }
}

void CompletionsCommand::addMemberCompletions(
  SuggestionContext& context,
  folly::dynamic& targets
) {
  int frameDepth = m_frameObj ? m_frameObj->m_frameDepth : 0;
  FramePointer fp(m_debugger, frameDepth);

  if (fp == nullptr) {
    return;
  }

  // Don't execute the evaluation if the input string looks like it might
  // contain a function call. This could have side effects we don't want to
  // cause during auto complete suggestions and it's better to be safe here.
  if (context.matchContext.find("(") != std::string::npos ||
      context.matchContext.find(")") != std::string::npos ||
      trimString(context.matchContext).size() == 0) {

    return;
  }

  DebuggerRequestInfo* ri = m_debugger->getRequestInfo();

  // SilentEvaluationContext suppresses all error output during the evaluation,
  // and re-enables it when it is destroyed.
  SilentEvaluationContext silentContext(m_debugger, ri);

  // Figure out what object the context for the completion refers to. Don't
  // hit any breakpoints during this eval.
  // NOTE: Prefixing the evaluation with @ to suppress any PHP notices that
  //  might otherwise be generated by evaluating the expression.
  context.matchContext = "<?hh return @(" + context.matchContext + ");";
  std::unique_ptr<Unit> unit(
                          compile_debugger_string(context.matchContext.c_str(),
                            context.matchContext.size(),
                            g_context->getRepoOptionsForCurrentFrame()));

  if (unit == nullptr) {
    // Expression failed to compile.
    return;
  }

  Unit* rawUnit = unit.get();
  ri->m_evaluationUnits.push_back(std::move(unit));
  const auto& result = g_context->evalPHPDebugger(
    rawUnit,
    frameDepth
  );

  if (result.failed) {
    return;
  }

  const Variant& obj = result.result;
  if (!obj.isObject()) {
    return;
  }

  const auto object = obj.toObject().get();
  if (object == nullptr) {
    return;
  }

  // Add any matching instance properties of the object.
  const Array instProps = object->toArray(false, true);
  for (ArrayIter iter(instProps); iter; ++iter) {
    std::string propName = iter.first().toString().toCppString();

    // The object's property name can be encoded with info about the modifier
    // and class. Decode it. (See HPHP::PreClass::manglePropName).
    if (propName[1] == '*') {
      // This is a protected property.
      propName = propName.substr(3);
    } else if (propName[0] == '\0') {
      // This is a private property on this object class or one of its base
      // classes.
      const unsigned long classNameEnd = propName.find('\0', 1);
      propName = propName.substr(classNameEnd + 1);
    }

    addIfMatch(propName, context.matchPrefix, CompletionTypeProp, targets);
  }

  // Add any instance methods of this object's class, or any of its parent
  // classes. NB parent methods are automatic, no need to walk the tree.
  Class* cls = object->getVMClass();
  int methodCount = cls->numMethods();
  for (Slot i = 0; i < methodCount; ++i) {
    const Func* method = cls->getMethod(i);
    if (method != nullptr && (method->attrs() & AttrStatic) == 0) {
      const std::string& name = method->name()->toCppString();
      addIfMatch(name, context.matchPrefix, CompletionTypeFn, targets);
    }
  }
}

void CompletionsCommand::addClassConstantCompletions(
  SuggestionContext& context,
  folly::dynamic& targets
) {
  HPHP::String classStr(context.matchContext.c_str());
  Class* cls = Class::load(classStr.get());
  if (cls == nullptr) {
    return;
  }

  // Add constants of this class. Note that here and in methods, we get
  // everything from this class and its ancestors
  for (Slot i = 0; i < cls->numConstants(); i++) {
    auto const &clsConst = cls->constants()[i];
    // constants() includes type constants and abstract constants, neither of
    // which are particularly useful for debugger completion
    if (clsConst.kind() == ConstModifiers::Kind::Value
        && !clsConst.isAbstractAndUninit()) {
      const std::string& name = clsConst.name->toCppString();
      addIfMatch(name, context.matchPrefix, CompletionTypeValue, targets);
    }
  }

  // Add static methods of this class.
  int methodCount = cls->numMethods();
  for (Slot i = 0; i < methodCount; ++i) {
    const Func* method = cls->getMethod(i);
    if (method != nullptr && method->attrs() & AttrStatic) {
      const std::string& name = method->name()->toCppString();
      addIfMatch(name, context.matchPrefix, CompletionTypeFn, targets);
    }
  }
}

void CompletionsCommand::addClassStaticCompletions(
  SuggestionContext& context,
  folly::dynamic& targets
) {
  HPHP::String classStr(context.matchContext.c_str());
  Class* cls = Class::load(classStr.get());

  while (cls != nullptr) {
    // Add static propreties of this class.
    const auto staticProperties = cls->staticProperties();
    for (Slot i = 0; i < cls->numStaticProperties(); i++) {
      const auto prop = staticProperties[i];
      const std::string& propName = prop.name.get()->toCppString();
      addIfMatch(propName, context.matchPrefix, CompletionTypeProp, targets);
    }

    cls = cls->parent();
  }
}

void CompletionsCommand::addIfMatch(
  const std::string& name,
  const std::string& matchPrefix,
  const char* type,
  folly::dynamic& targets
) {
  if (matchPrefix.empty() ||
      (name.size() >= matchPrefix.size() &&
       std::equal(
         matchPrefix.begin(),
         matchPrefix.end(),
         name.begin(),
         [](char c1, char c2) {
           return std::toupper(c1) == std::toupper(c2);
         }
       ))) {

    addCompletionTarget(
      targets,
      name.c_str(),
      type,
      matchPrefix.size()
    );
  }
}

void CompletionsCommand::addFuncConstantCompletions(
  SuggestionContext& context,
  folly::dynamic& targets
) {
  NamedFunc::foreach_cached_func([&](Func* func) {
    if (func->isGenerated()) return; //continue
    auto name = func->name()->toCppString();
    // Unit::getFunctions returns all lowercase names, lowercase here too.
    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    addIfMatch(name, context.matchPrefix, CompletionTypeFn, targets);
  });

  auto const consts = lookupDefinedConstants();
  IterateKV(consts.get(), [&] (TypedValue k, TypedValue) {
    auto const& name = String::attach(tvCastToStringData(k));
    addIfMatch(
      name.toCppString(),
      context.matchPrefix,
      CompletionTypeFn,
      targets
    );
  });

  NamedType::foreach_cached_class([&](Class* c) {
    if (!(c->attrs() & (AttrInterface | AttrTrait))) {
      auto const& name = c->name()->toCppString();
      addIfMatch(name, context.matchPrefix, CompletionTypeClass, targets);
    }
  });

  // Add PHP keywords.
  static const char* suggestionKeywords[] = {
    "__CLASS__",
    "__FILE__",
    "__FUNCTION__",
    "__LINE__",
    "__METHOD__",
    "abstract",
    "array",
    "as",
    "bool",
    "break",
    "case",
    "catch",
    "class",
    "clone",
    "const",
    "continue",
    "default",
    "do",
    "echo",
    "else",
    "elseif",
    "empty",
    "endfor",
    "endforeach",
    "endif",
    "endswitch",
    "eval",
    "exit",
    "extends",
    "final",
    "for",
    "foreach",
    "function",
    "if",
    "implements",
    "include",
    "include_once",
    "instanceof",
    "int",
    "interface",
    "isset",
    "list",
    "new",
    "null",
    "parent",
    "print",
    "private",
    "protected",
    "public",
    "require",
    "require_once",
    "return",
    "self"
    "static",
    "string",
    "switch",
    "throw",
    "try",
    "unset",
    "use",
    "while"
  };

  const auto count = sizeof(suggestionKeywords) / sizeof(suggestionKeywords[0]);

  for (int i = 0; i < count; i++) {
    addIfMatch(
      suggestionKeywords[i],
      context.matchPrefix,
      CompletionTypeKeyword,
      targets
    );
  }
}

}
}
