/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <lib/option.h>
#include <lib/parser/parser.h>
#include <util/logger.h>
#include <lib/analysis/analysis_result.h>
#include <lib/analysis/file_scope.h>
#include <lib/analysis/class_scope.h>
#include <lib/analysis/variable_table.h>
#include <lib/construct.h>
#include <lib/expression/assignment_expression.h>
#include <lib/expression/constant_expression.h>
#include <lib/expression/unary_op_expression.h>
#include <lib/expression/scalar_expression.h>
#include <lib/expression/array_pair_expression.h>
#include <lib/parser/parser.h>
#include <lib/parser/hphp.tab.hpp>
#include <lib/expression/expression_list.h>
#include <util/db_query.h>
#include <boost/algorithm/string/trim.hpp>
#include <util/util.h>

using namespace HPHP;
using namespace std;
using namespace boost;

///////////////////////////////////////////////////////////////////////////////

#define LOAD_OPTION(s) \
  if ((v = GetValue(variables, #s)) && !Load(s, v)) return false;

bool Option::Load(VariableTablePtr variables) {
  ExpressionPtr v;

  LOAD_OPTION(PackageDirectories);
  LOAD_OPTION(PackageFiles);
  LOAD_OPTION(PackageExcludeDirs);
  LOAD_OPTION(PackageExcludeFiles);
  LOAD_OPTION(PackageExcludeStaticFiles);
  LOAD_OPTION(AllowedBadPHPIncludes);
  LOAD_OPTION(IncludeRoots);
  LOAD_OPTION(AutoloadRoots);
  LOAD_OPTION(IncludePaths);
  LOAD_OPTION(DefaultIncludeRoot);
  LOAD_OPTION(DynamicFunctionCalls);
  LOAD_OPTION(GeneratePickledPHP);
  LOAD_OPTION(GenerateInlinedPHP);
  LOAD_OPTION(GenerateTrimmedPHP);
  LOAD_OPTION(GenerateInlineComments);
  LOAD_OPTION(GenerateInferredTypes);
  LOAD_OPTION(ConvertSuperGlobals);
  LOAD_OPTION(ConvertQOpExpressions);
  LOAD_OPTION(ProgramPrologue);
  LOAD_OPTION(TrimmedPrologue);
  LOAD_OPTION(DynamicFunctionPrefixes);
  LOAD_OPTION(DynamicFunctionPostfixes);
  LOAD_OPTION(DynamicMethodPrefixes);
  LOAD_OPTION(DynamicMethodPostfixes);
  LOAD_OPTION(DynamicClassPrefixes);
  LOAD_OPTION(DynamicClassPostfixes);
  LOAD_OPTION(DynamicInvokeFunctions);
  LOAD_OPTION(IdPrefix);
  LOAD_OPTION(LambdaPrefix);
  LOAD_OPTION(Tab);
  LOAD_OPTION(GenerateCPPMacros);
  LOAD_OPTION(GenerateCPPMain);
  LOAD_OPTION(GenerateCPPComments);
  LOAD_OPTION(GenerateCPPMetaInfo);
  LOAD_OPTION(GenerateCPPNameSpace);
  LOAD_OPTION(ConditionalIncludeExpandLevel);
  LOAD_OPTION(DependencyMaxProgram);
  LOAD_OPTION(CodeErrorMaxProgram);
  return true;
}

///////////////////////////////////////////////////////////////////////////////

std::string Option::SystemRoot;
std::string Option::RootDirectory;
set<string> Option::PackageDirectories;
set<string> Option::PackageFiles;
set<string> Option::PackageExcludeDirs;
set<string> Option::PackageExcludeFiles;
set<string> Option::PackageExcludeStaticFiles;
bool Option::CachePHPFile = false;

set<string> Option::AllowedBadPHPIncludes;
map<string, string> Option::IncludeRoots;
map<string, string> Option::AutoloadRoots;
vector<string> Option::IncludePaths;
string Option::DefaultIncludeRoot;
map<string, int> Option::DynamicFunctionCalls;

bool Option::GeneratePickledPHP = false;
bool Option::GenerateInlinedPHP = false;
bool Option::GenerateTrimmedPHP = false;
bool Option::GenerateInlineComments = true;
bool Option::GenerateInferredTypes = false;
bool Option::ConvertSuperGlobals = false;
bool Option::ConvertQOpExpressions = false;
string Option::ProgramPrologue;
string Option::TrimmedPrologue;
vector<string> Option::DynamicFunctionPrefixes;
vector<string> Option::DynamicFunctionPostfixes;
vector<string> Option::DynamicMethodPrefixes;
vector<string> Option::DynamicMethodPostfixes;
vector<string> Option::DynamicClassPrefixes;
vector<string> Option::DynamicClassPostfixes;
set<string> Option::DynamicInvokeFunctions;
string Option::IdPrefix = "$$";
string Option::LambdaPrefix = "df_";
string Option::Tab = "  ";

/**
 * They all have to be something different. Otherwise, there is always a chance
 * of name collision or incorrect code transformation.
 */
const char *Option::FunctionPrefix = "f_";
const char *Option::BuiltinFunctionPrefix = "x_";
const char *Option::InvokePrefix = "i_";
const char *Option::CreateObjectPrefix = "co_";
const char *Option::PseudoMainPrefix = "pm_";
const char *Option::VariablePrefix = "v_";
const char *Option::GlobalVariablePrefix = "gv_";
const char *Option::StaticVariablePrefix = "sv_";
const char *Option::ScalarArrayName = "sa_";
const char *Option::SystemScalarArrayName = "ssa_";
const char *Option::ClassPrefix = "c_";
const char *Option::ClassStaticsPrefix = "cs_";
const char *Option::ClassStaticsObjectPrefix = "cso_";
const char *Option::ClassStaticsIdGetterPrefix = "csig_";
const char *Option::ClassStaticInitializerPrefix = "csi_";
const char *Option::ClassStaticInitializerFlagPrefix = "csf_";
const char *Option::ClassWrapperFunctionPrefix = "cw_";
const char *Option::ObjectPrefix = "o_";
const char *Option::ObjectStaticPrefix = "os_";
const char *Option::SmartPtrPrefix = "p_";
const char *Option::MethodPrefix = "t_";
const char *Option::MethodImplPrefix = "ti_";
const char *Option::PropertyPrefix = "m_";
const char *Option::StaticPropertyPrefix = "s_";
const char *Option::ConstantPrefix = "k_";
const char *Option::ClassConstantPrefix = "q_";
const char *Option::ExceptionPrefix = "e_";
const char *Option::TempVariablePrefix = "r_";
const char *Option::EvalOrderTempPrefix = "eo_";
const char *Option::EvalInvokePrefix = "ei_";

const char *Option::FFIFnPrefix = "ffi_";

const char *Option::TempPrefix = "tmp";
const char *Option::MapPrefix = "map";
const char *Option::IterPrefix = "iter";
const char *Option::InitPrefix = "inited_";

const char *Option::SystemFilePrefix = "sys/";
const char *Option::UserFilePrefix = "php/";
const char *Option::ClassHeaderPrefix = "cls/";
const char *Option::ClusterPrefix = "cpp/";
const char *Option::FFIFilePrefix = "ffi/";

bool Option::PreOptimization = false;
bool Option::PostOptimization = false;
bool Option::ScalarArrayOptimization = true;
bool Option::ScalarArrayCompression = true;
bool Option::LiteralStringCompression = false;
int Option::ScalarArrayFileCount = 1;
int Option::ScalarArrayOverflowLimit = 2000;
bool Option::SeparateCompilation = false;
bool Option::SeparateCompLib = false;
int Option::LiteralStringFileCount = 50;

std::string Option::RTTIOutputFile;
std::string Option::RTTIDirectory;
bool Option::GenRTTIProfileData = false;
bool Option::UseRTTIProfileData = false;

bool Option::StaticMethodAutoFix = false;

bool Option::GenerateCPPMacros = true;
bool Option::GenerateCPPMain = false;
bool Option::GenerateCPPComments = true;
bool Option::GenerateCPPMetaInfo = true;
bool Option::GenerateCPPNameSpace = true;
bool Option::KeepStatementsWithNoEffect = false;

int Option::ConditionalIncludeExpandLevel = 1;

int Option::DependencyMaxProgram = 1;
int Option::CodeErrorMaxProgram = 1;

bool Option::GenerateFFI = false;
Option::EvalLevel Option::EnableEval = NoEval;

std::string Option::JavaFFIRootPackage;

std::string Option::ProgramName;

bool Option::EnableXHP = false;

int Option::InvokeFewArgsCount = 6;
bool Option::PrecomputeLiteralStrings = false;
bool Option::FlattenInvoke = true;
int Option::InlineFunctionThreshold = -1;
bool Option::ControlEvalOrder = true;

bool Option::AllDynamic = false;
bool Option::AllVolatile = false;

std::string Option::FlibDirectory;

///////////////////////////////////////////////////////////////////////////////
// load from a PHP file

std::string Option::GetSystemRoot() {
  if (SystemRoot.empty()) {
    const char *home = getenv("HPHP_HOME");
    if (!home || !*home) {
      throw FatalErrorException("Environment variable HPHP_HOME is not set.");
    }
    SystemRoot = home;
    SystemRoot += "/src";
  }
  return SystemRoot;
}

bool Option::Load(const char *filename) {
  ASSERT(filename && *filename);
  try {
    Scanner scanner(new ylmm::basic_buffer(filename), true, false);
    Logger::Info("loading options from %s...", filename);
    AnalysisResultPtr ar(new AnalysisResult());

    struct stat sb;
    if (stat(filename, &sb)) {
      Logger::Error("Unable to stat file %s", filename);
      return false;
    }
    ParserPtr parser(new Parser(scanner, filename, sb.st_size, ar));
    if (parser->parse()) {
      Logger::Error("Unable to parse file: %s\n%s", filename,
                    parser->getMessage().c_str());
      return false;
    }

    FileScopePtr file = ar->findFileScope(filename, false);
    ClassScopePtr cls = file->getClass("hphpoption");
    if (!cls) {
      Logger::Error("Unable to find HPHPOption class in %s", filename);
      return false;
    }

    if (!Load(cls->getVariables())) {
      return false;
    }

  } catch (std::runtime_error) {
    Logger::Error("Unable to open file %s", filename);
    return false;
  }
  return true;
}

ExpressionPtr Option::GetValue(VariableTablePtr variables,
                               const char *varName) {
  ConstructPtr decl = variables->getDeclaration(varName);
  if (!decl) {
    return ExpressionPtr();
  }

  AssignmentExpressionPtr assignment =
    dynamic_pointer_cast<AssignmentExpression>(decl);
  if (!assignment) {
    Logger::Error("Line %d: Ignored option %s", decl->getLocation()->line1,
                  varName);
    return ExpressionPtr();
  }

  return assignment->getValue();
}

bool Option::GetArrayElements(ExpressionPtr value, ExpressionListPtr &out) {
  UnaryOpExpressionPtr v = dynamic_pointer_cast<UnaryOpExpression>(value);
  if (!v || v->getOp() != T_ARRAY) {
    Logger::Error("Line %d: invalid array: %s", value->getLocation()->line1,
                  value->getText().c_str());
    return false;
  }

  ExpressionPtr exp = v->getExpression();
  out = dynamic_pointer_cast<ExpressionList>(exp);
  if (!out) {
    Logger::Error("Line %d: invalid array: %s", value->getLocation()->line1,
                  value->getText().c_str());
    return false;
  }

  return true;
}

bool Option::Load(bool &option, ExpressionPtr value) {
  ConstantExpressionPtr v = dynamic_pointer_cast<ConstantExpression>(value);
  if (!v || !v->isBoolean()) {
    Logger::Error("Line %d: invalid boolean: %s", value->getLocation()->line1,
                  value->getText().c_str());
    return false;
  }
  option = v->getBooleanValue();
  return true;
}

bool Option::Load(int &option, ExpressionPtr value) {
  ScalarExpressionPtr v = dynamic_pointer_cast<ScalarExpression>(value);
  if (!v || !v->isLiteralInteger()) {
    Logger::Error("Line %d: invalid integer: %s", value->getLocation()->line1,
                  value->getText().c_str());
    return false;
  }
  option = v->getLiteralInteger();
  return true;
}

bool Option::Load(string &option, ExpressionPtr value) {
  ScalarExpressionPtr v = dynamic_pointer_cast<ScalarExpression>(value);
  if (!v || !v->isLiteralString()) {
    Logger::Error("Line %d: invalid string: %s", value->getLocation()->line1,
                  value->getText().c_str());
    return false;
  }
  option = v->getLiteralString();
  return true;
}

bool Option::Load(set<string> &option, ExpressionPtr value) {
  ExpressionListPtr elements;
  if (!GetArrayElements(value, elements)) return false;

  for (int i = 0; i < elements->getCount(); i++) {
    ExpressionPtr e = (*elements)[i];
    ArrayPairExpressionPtr pair = dynamic_pointer_cast<ArrayPairExpression>(e);

    ScalarExpressionPtr v;
    if (pair) v = dynamic_pointer_cast<ScalarExpression>(pair->getValue());

    if (!pair || !v || !v->isLiteralString()) {
      Logger::Error("Line %d: invalid element: %s", e->getLocation()->line1,
                    e->getText().c_str());
      return false;
    }
    option.insert(v->getLiteralString());
  }

  return true;
}

bool Option::Load(map<string, string> &option, ExpressionPtr value) {
  ExpressionListPtr elements;
  if (!GetArrayElements(value, elements)) return false;

  for (int i = 0; i < elements->getCount(); i++) {
    ExpressionPtr e = (*elements)[i];
    ArrayPairExpressionPtr pair = dynamic_pointer_cast<ArrayPairExpression>(e);

    ScalarExpressionPtr n, v;
    if (pair) n = dynamic_pointer_cast<ScalarExpression>(pair->getName());
    if (pair) v = dynamic_pointer_cast<ScalarExpression>(pair->getValue());

    if (!pair || !n || !v || !n->isLiteralString() || !v->isLiteralString()) {
      Logger::Error("Line %d: invalid element: %s", e->getLocation()->line1,
                    e->getText().c_str());
      return false;
    }
    option[n->getLiteralString()] = v->getLiteralString();
  }

  return true;
}

bool Option::Load(map<string, int> &option, ExpressionPtr value) {
  ExpressionListPtr elements;
  if (!GetArrayElements(value, elements)) return false;

  for (int i = 0; i < elements->getCount(); i++) {
    ExpressionPtr e = (*elements)[i];
    ArrayPairExpressionPtr pair = dynamic_pointer_cast<ArrayPairExpression>(e);

    bool negative = false;
    ScalarExpressionPtr n, v;
    if (pair) n = dynamic_pointer_cast<ScalarExpression>(pair->getName());
    if (pair) {
      if (pair->getValue()->is(Expression::KindOfUnaryOpExpression)) {
        UnaryOpExpressionPtr una =
          dynamic_pointer_cast<UnaryOpExpression>(pair->getValue());
        if (una->getOp() != '+' && una->getOp() != '-') {
          Logger::Error("Line %d: invalid integer: %s",
                        una->getLocation()->line1, una->getText().c_str());
          return false;
        }

        v = dynamic_pointer_cast<ScalarExpression>(una->getExpression());
        if (una->getOp() == '-') {
          negative = true;
        }
      } else {
        v = dynamic_pointer_cast<ScalarExpression>(pair->getValue());
      }
    }

    if (!pair || !n || !v || !n->isLiteralString() || !v->isLiteralInteger()) {
      Logger::Error("Line %d: invalid element: %s", e->getLocation()->line1,
                    e->getText().c_str());
      return false;
    }

    if (negative) {
      option[n->getLiteralString()] = - v->getLiteralInteger();
    } else {
      option[n->getLiteralString()] = v->getLiteralInteger();
    }
  }

  return true;
}

bool Option::Load(vector<string> &option, ExpressionPtr value) {
  ExpressionListPtr elements;
  if (!GetArrayElements(value, elements)) return false;

  for (int i = 0; i < elements->getCount(); i++) {
    ExpressionPtr e = (*elements)[i];
    ArrayPairExpressionPtr pair = dynamic_pointer_cast<ArrayPairExpression>(e);

    ScalarExpressionPtr v;
    if (pair) v = dynamic_pointer_cast<ScalarExpression>(pair->getValue());

    if (!pair || !v || !v->isLiteralString()) {
      Logger::Error("Line %d: invalid element: %s", e->getLocation()->line1,
                    e->getText().c_str());
      return false;
    }
    option.push_back(v->getLiteralString());
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// load from HDF file

void Option::LoadRootHdf(const Hdf &roots, map<string, string> &map) {
  if (roots.exists()) {
    for (Hdf hdf = roots.firstChild(); hdf.exists(); hdf = hdf.next()) {
      map[hdf["root"].get()] = hdf["path"].get();
    }
  }
}

void Option::LoadRootHdf(const Hdf &roots, vector<string> &vec) {
  if (roots.exists()) {
    for (Hdf hdf = roots.firstChild(); hdf.exists(); hdf = hdf.next()) {
      vec.push_back(hdf.getString(""));
    }
  }
}

void Option::Load(Hdf &config) {
  LoadRootHdf(config["IncludeRoots"], IncludeRoots);
  LoadRootHdf(config["AutoloadRoots"], AutoloadRoots);
  LoadRootHdf(config["IncludePaths"], IncludePaths);

  config["PackageDirectories"].get(PackageDirectories);
  config["PackageExcludeDirs"].get(PackageExcludeDirs);
  config["PackageExcludeFiles"].get(PackageExcludeFiles);
  config["PackageExcludeStaticFiles"].get(PackageExcludeStaticFiles);
  CachePHPFile = config["CachePHPFile"].getBool(false);
  config["DynamicFunctionPrefix"].get(DynamicFunctionPrefixes);
  config["DynamicFunctionPostfix"].get(DynamicFunctionPostfixes);
  config["DynamicMethodPrefix"].get(DynamicMethodPrefixes);
  config["DynamicInvokeFunctions"].get(DynamicInvokeFunctions);

  ScalarArrayFileCount = config["ScalarArrayFileCount"].getByte(1);
  if (ScalarArrayFileCount <= 0) ScalarArrayFileCount = 1;
  LiteralStringFileCount = config["LiteralStringFileCount"].getInt32(50);
  if (LiteralStringFileCount <= 0) LiteralStringFileCount = 50;
  ScalarArrayOverflowLimit = config["ScalarArrayOverflowLimit"].getInt32(2000);
  if (ScalarArrayOverflowLimit <= 0) ScalarArrayOverflowLimit = 2000;
  FlibDirectory = config["FlibDirectory"].getString();
  EnableXHP = config["EnableXHP"].getBool();
  RTTIOutputFile = config["RTTIOutputFile"].getString();
  EnableEval = (EvalLevel)config["EnableEval"].getByte(0);
  AllDynamic = config["AllDynamic"].getBool();
  AllVolatile = config["AllVolatile"].getBool();
}

///////////////////////////////////////////////////////////////////////////////
/**
 * Load from database "config" table that looks like this:
 *
 * CREATE TABLE `config` (
 *   `id` varchar(255) NOT NULL,
 *   `name` varchar(255) NOT NULL,
 *   `value` varchar(255) NOT NULL,
 *   PRIMARY KEY  (`id`,`name`)
 * ) ENGINE=InnoDB DEFAULT CHARSET=latin1;
 *
 * Only a subset of options are supported.
 */
void Option::Load(ServerDataPtr server) {
  DBConn conn;
  conn.open(server);

  DBQueryPtr q(new DBQuery(&conn, "SELECT id, name, value FROM config"));
  DBDataSet ds;
  q->execute(ds);

  for (ds.moveFirst(); ds.getRow(); ds.moveNext()) {
    const char *id = ds.getField(0);
    string name = ds.getField(1);
    string value = ds.getField(2);
    boost::algorithm::trim<std::string>(name);
    boost::algorithm::trim<std::string>(value);

    if (strcmp(id, "IncludeRoots") == 0) {
      IncludeRoots[name] = value;
    } else if (strcmp(id, "AutoloadRoots") == 0) {
      AutoloadRoots[name] = value;
    } else if (strcmp(id, "IncludePaths") == 0) {
      IncludePaths.push_back(name);
    } else if (strcmp(id, "PackageDirectories") == 0) {
      PackageDirectories.insert(name);
    } else if (strcmp(id, "PackageExcludeDirs") == 0) {
      PackageExcludeDirs.insert(name);
    } else if (strcmp(id, "PackageExcludeFiles") == 0) {
      PackageExcludeFiles.insert(name);
    } else if (strcmp(id, "PackageExcludeStaticFiles") == 0) {
      PackageExcludeStaticFiles.insert(name);
    } else if (strcmp(id, "DynamicFunctionPrefix") == 0) {
      DynamicFunctionPrefixes.push_back(name);
    } else if (strcmp(id, "DynamicFunctionPostfix") == 0) {
      DynamicFunctionPostfixes.push_back(name);
    } else if (strcmp(id, "DynamicMethodPrefix") == 0) {
      DynamicMethodPrefixes.push_back(name);
    } else if (strcmp(id, "DynamicInvokeFunctions") == 0) {
      DynamicInvokeFunctions.insert(name);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

bool Option::isDynamicFunction(bool method, const std::string &name) {
  if (method) {
    return isDynamic(name, DynamicMethodPrefixes, DynamicMethodPostfixes);
  }
  return isDynamic(name, DynamicFunctionPrefixes, DynamicFunctionPostfixes);
}

bool Option::isDynamicClass(const std::string &name) {
  return isDynamic(name, DynamicClassPrefixes, DynamicClassPostfixes);
}

bool Option::isDynamic(const std::string &name,
                       const std::vector<std::string> &prefixes,
                       const std::vector<std::string> &postfixes) {
  if (name.substr(0, 4) == "dyn_") return true;

  for (unsigned int i = 0; i < prefixes.size(); i++) {
    const string &prefix = prefixes[i];
    if (name.substr(0, prefix.length()) == prefix) {
      return true;
    }
  }

  for (unsigned int i = 0; i < postfixes.size(); i++) {
    const string &postfix = postfixes[i];
    if (name.length() > postfix.length() &&
        name.substr(name.length() - postfix.length()) == postfix) {
      return true;
    }
  }

  return false;
}

std::string Option::getAutoloadRoot(const std::string &name) {
  for (map<string, string>::const_iterator iter = AutoloadRoots.begin();
       iter != AutoloadRoots.end(); ++iter) {
    if (name.substr(0, iter->first.length()) == iter->first) {
      return iter->second;
    }
  }
  return "";
}

std::string Option::mangleFilename(const std::string &name, bool id) {
  string ret = UserFilePrefix;
  ret += name;

  if (id) {
    Util::replaceAll(ret, "/", "$");
    Util::replaceAll(ret, "-", "_");
    Util::replaceAll(ret, ".", "_");
  }
  return ret;
}

std::string Option::FormatClusterFile(int index) {
  char buf[PATH_MAX];
  snprintf(buf, sizeof(buf), "%s%03d", ClusterPrefix, index);
  return buf;
}
