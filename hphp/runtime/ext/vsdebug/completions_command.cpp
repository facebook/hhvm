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

#include "hphp/parser/scanner.h"
#include "hphp/runtime/base/tv-variant.h"
#include "hphp/runtime/ext/vsdebug/command.h"
#include "hphp/runtime/ext/vsdebug/debugger.h"
#include "hphp/runtime/vm/runtime-compiler.h"
#include "hphp/runtime/vm/vm-regs.h"

namespace HPHP {
namespace VSDEBUG {

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

  // The tokenizer requires the expression to look like inline PHP, which must
  // start with <?
  auto it = expr.find("<?");
  if (it != 0) {
    text = "<? " + text;
  }

  // Use the PHP tokenizer to determine what the string we were given most
  // likely contains. This will tell us what sort of completions we should
  // be trying to recommend.
  Scanner scanner(
    text.c_str(),
    text.size(),
    RuntimeOption::GetScannerType() | Scanner::ReturnAllTokens
  );

  ScannerToken token;
  Location location;
  int tokenId;
  std::vector<TokenEntry> processedTokens;

  while ((tokenId = scanner.getNextToken(token, location))) {
    TokenEntry te;
    te.tokenType = tokenId;
    te.tokenValue = token.text();

    // Convert token position to 0-based.
    te.tokenPosition = location.r.char0 - 1;
    processedTokens.push_back(te);
  }

  // Start with the last token and search backwards through the token types
  // to determine what sort of completion context we are in.
  if (processedTokens.size() > 0) {
    TokenEntry trailingToken =
      processedTokens[processedTokens.size() - 1];

    int trailingTokenType = trailingToken.tokenType;
    TokenEntry preceedingToken;
    int preceedingTokenType = T_CLOSE_TAG;
    if (processedTokens.size() > 1) {
      preceedingToken = processedTokens[processedTokens.size() - 2];
      preceedingTokenType = preceedingToken.tokenType;
    }

    context.matchPrefix = trailingToken.tokenValue;
    context.matchContext = getCompletionContext(text, processedTokens);

    if (trailingTokenType == T_VARIABLE || trailingTokenType == (int)'$') {
      if (preceedingTokenType == T_DOUBLE_COLON) {
        // A static variable reference {classname}::${variable}
        context.type = SuggestionType::ClassStatic;
      } else {
        // A simple variable name of the from ${variable}.
        context.type = SuggestionType::Variable;
      }
    } else if (trailingTokenType == T_STRING) {
      if (preceedingTokenType == T_DOUBLE_COLON) {
        // A variable constant of the form {classname}::{constant}
        // Constant is a string, so it could be an actual class constant,
        // or the name of a static method on the class.
        context.type = SuggestionType::ClassConstant;
      } else if (preceedingTokenType == T_OBJECT_OPERATOR) {
        // A property dereference of the form ${expr}->{text}
        //   Note: {expr} could be a simple variable name, or a more
        //   complicated expression or property chain, including
        //   $foo->bar->{text} or even $foo[3]->bar[2]->{text}, etc...
        //
        //   Suggest instances members of the object referred to by the left
        //   of the -> operator, as well as instances methods of the same
        //   object.
        context.type = SuggestionType::Member;
      } else {
        // Expression ends in a string. Suggest function names and constants.
        context.type = SuggestionType::FuncsAndConsts;
      }
    } else if (trailingTokenType == T_OBJECT_OPERATOR) {
      // Ending with ->, try to match all members of the context object.
      context.type = SuggestionType::Member;
      context.matchPrefix = "";
    } else if (trailingTokenType == T_DOUBLE_COLON) {
      // Ending with ::, try to match all members of the context class.
      context.type = SuggestionType::ClassConstant;
      context.matchPrefix = "";
    }
  }

  // No suggestions to make for this input.
  return context;
}

std::string CompletionsCommand::getCompletionContext(
  std::string& text,
  std::vector<TokenEntry>& processedTokens
) {
  int size = processedTokens.size();
  if (size < 2) {
    return "";
  }

  // Search backwards from where the context is going to start until
  // we reach the beginning of the input or find a whitespace token.
  int searchStartIndex =
    processedTokens[size - 1].tokenType == T_OBJECT_OPERATOR ||
    processedTokens[size - 1].tokenType == T_DOUBLE_COLON
      ? size - 1
      : size - 2;

  int startToken;
  for (startToken = searchStartIndex; startToken > 0; startToken--) {
    TokenEntry te = processedTokens[startToken];
    if (te.tokenType == T_WHITESPACE ||
        te.tokenType == (int)',' ||
        te.tokenType == (int)'(') {

      startToken++;
      break;
    }
  }

  if (startToken >= searchStartIndex || startToken < 0) {
    return "";
  }

  int startPos = processedTokens[startToken].tokenPosition;
  int length = processedTokens[searchStartIndex].tokenPosition - startPos;
  return text.substr(startPos, length);
}


bool CompletionsCommand::executeImpl(
  DebuggerSession* session,
  folly::dynamic* responseMsg
) {
  VMRegAnchor regAnchor;

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

    // Sort results, prefer shorter strings, then alphabatize.
    std::sort(
      targets.begin(),
      targets.end(),
      [&](folly::dynamic& a, folly::dynamic& b) {
        const std::string& strA = tryGetString(a, "label", "");
        const std::string& strB = tryGetString(b, "label", "");
        size_t sizeA = strA.size();
        size_t sizeB = strB.size();
        if (sizeA == sizeB) {
          // Alphabatize the rest.
          return strA.compare(strB) <= 0;
        } else {
          return sizeA < sizeB;
        }
      }
    );

    // Eliminate any duplicates: for example, an object might actually have
    // multiple props with the same name if there is inheritance of private
    // members from a base class.
    targets.erase(std::unique(targets.begin(), targets.end()), targets.end());
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
  if (m_frameObj == nullptr) {
    return;
  }

  auto const fp = g_context->getFrameAtDepth(m_frameObj->m_frameDepth);
  if (fp == nullptr || fp->func() == nullptr) {
    return;
  }

  // If there is a $this, add it.
  if (fp->func()->cls() != nullptr && fp->hasThis()) {
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
  if (m_frameObj == nullptr) {
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

  RequestInfo* ri = m_debugger->getRequestInfo();

  // SilentEvaluationContext suppresses all error output during the evaluation,
  // and re-enables it when it is destroyed.
  SilentEvaluationContext silentContext(m_debugger, ri);

  // Figure out what object the context for the completion refers to. Don't
  // hit any breakpoints during this eval.
  // NOTE: Prefixing the evaluation with @ to suppress any PHP notices that
  //  might otherwise be generated by evaluating the expression.
  context.matchContext = "<?hh return @(" + context.matchContext + ");";
  std::unique_ptr<Unit> unit(
    compile_string(context.matchContext.c_str(), context.matchContext.size()));

  if (unit == nullptr) {
    // Expression failed to compile.
    return;
  }

  const auto& result = g_context->evalPHPDebugger(
    unit.get(),
    m_frameObj->m_frameDepth
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
  const Array instProps = object->toArray();
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
  // classes.
  Class* cls = object->getVMClass();
  while (cls != nullptr) {
    int methodCount = cls->numMethods();
    for (Slot i = 0; i < methodCount; ++i) {
      const Func* method = cls->getMethod(i);
      if (method != nullptr && (method->attrs() & AttrStatic) == 0) {
        const std::string& name = method->name()->toCppString();
        addIfMatch(name, context.matchPrefix, CompletionTypeFn, targets);
      }
    }

    cls = cls->parent();
  }
}

void CompletionsCommand::addClassConstantCompletions(
  SuggestionContext& context,
  folly::dynamic& targets
) {
  HPHP::String classStr(context.matchContext.c_str());
  Class* cls = Unit::lookupClass(classStr.get());

  while (cls != nullptr) {
    for (Slot i = 0; i < cls->numConstants(); i++) {
      const std::string& name = cls->constants()[i].name->toCppString();
      addIfMatch(name, context.matchPrefix, CompletionTypeValue, targets);
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

    cls = cls->parent();
  }
}

void CompletionsCommand::addClassStaticCompletions(
  SuggestionContext& context,
  folly::dynamic& targets
) {
  HPHP::String classStr(context.matchContext.c_str());
  Class* cls = Unit::lookupClass(classStr.get());

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
  auto systemFuncs = Unit::getSystemFunctions();
  auto userFuncs = Unit::getUserFunctions();
  auto consts = lookupDefinedConstants();

  for (ArrayIter iter(systemFuncs); iter; ++iter) {
    const std::string& name = iter.second().toString().toCppString();
    addIfMatch(name, context.matchPrefix, CompletionTypeFn, targets);
  }

  for (ArrayIter iter(userFuncs); iter; ++iter) {
    const std::string& name = iter.second().toString().toCppString();
    addIfMatch(name, context.matchPrefix, CompletionTypeFn, targets);
  }

  for (ArrayIter iter(consts); iter; ++iter) {
    const std::string& name = iter.first().toString().toCppString();
    addIfMatch(name, context.matchPrefix, CompletionTypeFn, targets);
  }

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
    "declare",
    "default",
    "do",
    "double",
    "echo",
    "else",
    "elseif",
    "empty",
    "enddeclare",
    "endfor",
    "endforeach",
    "endif",
    "endswitch",
    "endwhile",
    "eval",
    "exit",
    "extends",
    "final",
    "for",
    "foreach",
    "function",
    "global",
    "halt_compiler",
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
    "object",
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
    "var",
    "while"
  };

  for (int i = 0; i < ARRAY_SIZE(suggestionKeywords); i++) {
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
