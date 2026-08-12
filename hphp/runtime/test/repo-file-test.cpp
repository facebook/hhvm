/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license, please send a note |
   | to license@php.net so we can mail you a copy immediately.            |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/vm/repo-file.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/constant.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/util/configs/server.h"

#include <folly/ScopeGuard.h>
#include <folly/testing/TestUtil.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace HPHP {
namespace {

using ::testing::AllOf;
using ::testing::HasSubstr;
using ::testing::ThrowsMessage;

struct UnitSpec {
  std::string path;
  std::string fatalMessage;
  std::string constantName;
  std::string methCallerName;
  std::string secondMethCallerName;
  std::string functionName;
  std::string typeName;
  std::string typeAliasName;
  std::string moduleName;
};

void finishRepo(RepoFileBuilder& builder,
                const std::vector<UnitSpec>& units,
                uint64_t signature,
                const PackageInfo* packageInfo = nullptr,
                RepoFileData* base = nullptr,
                const std::vector<std::string>& excludedPaths = {},
                bool reverseUnitSns = false) {
  RepoAutoloadMapBuilder autoload;

  for (size_t i = 0; i < units.size(); ++i) {
    auto unit = createFatalUnit(
      makeStaticString(units[i].path),
      SHA1{static_cast<uint64_t>(i + 1)},
      FatalOp::Runtime,
      units[i].fatalMessage,
      {}
    );
    unit->m_sn = reverseUnitSns ? units.size() - 1 - i : i;

    if (!units[i].constantName.empty()) {
      Constant constant;
      constant.name = makeStaticString(units[i].constantName);
      constant.val = make_tv<KindOfInt64>(i);
      constant.attrs = AttrNone;
      unit->addConstant(constant);
    }

    if (!units[i].typeName.empty()) {
      auto const type = unit->newPreClassEmitter(units[i].typeName);
      type->init(
        1, 1, AttrNone, staticEmptyString(), staticEmptyString(), false);
    }
    if (!units[i].typeAliasName.empty()) {
      auto typeStructure = make_dict_array("kind", 1);
      typeStructure.setEvalScalar();
      auto const typeAlias =
        unit->newTypeAliasEmitter(units[i].typeAliasName);
      typeAlias->init(
        1,
        1,
        AttrNone,
        TypeConstraint{},
        AliasKind::TypeAlias,
        std::move(typeStructure),
        Array{}
      );
    }
    if (!units[i].moduleName.empty()) {
      unit->addModule(Module{
        makeStaticString(units[i].moduleName),
        staticEmptyString(),
        1,
        1,
        AttrNone,
        {},
      });
    }

    auto const addFunction = [&](const std::string& name, Attr attrs) {
      if (name.empty()) {
        return;
      }
      auto const func =
        unit->newFuncEmitter(makeStaticString(name));
      func->init(1, 1, attrs, nullptr, false);
      func->retTypeConstraints =
        TypeIntersectionConstraint{TypeConstraint{}};
      func->recordSourceLocation(Location::Range{1, 1, 1, 1}, 0);
      func->emitOp(Op::Null);
      func->emitOp(Op::RetC);
      func->finish();
    };
    addFunction(units[i].functionName, AttrNone);
    addFunction(units[i].methCallerName, AttrIsMethCaller);
    addFunction(units[i].secondMethCallerName, AttrIsMethCaller);

    autoload.addUnit(*unit);
    builder.add(*unit);
  }

  auto const& currentPackageInfo =
    packageInfo ? *packageInfo : RepoOptions::defaults().packageInfo();
  if (base) {
    builder.addFrom(*base, autoload, excludedPaths, currentPackageInfo);
  }

  RepoGlobalData globalData{};
  globalData.Signature = signature;
  builder.finish(
    globalData,
    autoload,
    currentPackageInfo
  );
}

void buildRepo(const std::string& path,
               const std::vector<UnitSpec>& units,
               uint64_t signature,
               bool enableUnitEmitterReuse = false) {
  RepoFileBuilder builder{path, enableUnitEmitterReuse};
  finishRepo(builder, units, signature);
}

void buildIncrementalRepo(const std::string& path,
                          const std::string& basePath,
                          const std::vector<UnitSpec>& units,
                          std::vector<std::string> invalidatedPaths,
                          uint64_t signature,
                          bool reverseUnitSns = false) {
  for (auto const& unit : units) {
    auto path = unit.path;
    if (!Cfg::Server::SourceRoot.empty() &&
        path.starts_with(Cfg::Server::SourceRoot)) {
      path.erase(0, Cfg::Server::SourceRoot.size());
    }
    invalidatedPaths.push_back(std::move(path));
  }
  RepoFileData base{basePath};
  RepoFileBuilder builder{path, true};
  finishRepo(
    builder, units, signature, nullptr, &base, invalidatedPaths,
    reverseUnitSns);
}

std::string loadFatalMessage(const std::string& path) {
  auto unit = RepoFile::loadUnitEmitter(makeStaticString(path), nullptr, false);
  EXPECT_NE(unit, nullptr);
  return unit ? unit->m_fatalMsg : std::string{};
}

std::vector<std::string> loadConstantNames(const std::string& path) {
  auto const symbols = RepoFile::findUnitSymbols(makeStaticString(path));
  EXPECT_NE(symbols, nullptr);

  std::vector<std::string> names;
  if (symbols) {
    for (auto const& [name, type] : *symbols) {
      if (type == RepoSymbolType::CONSTANT) {
        names.push_back(name->toCppString());
      }
    }
  }
  return names;
}

std::vector<std::string> loadFunctionNames(const std::string& path) {
  auto const symbols = RepoFile::findUnitSymbols(makeStaticString(path));
  EXPECT_NE(symbols, nullptr);

  std::vector<std::string> names;
  if (symbols) {
    for (auto const& [name, type] : *symbols) {
      if (type == RepoSymbolType::FUNC) {
        names.push_back(name->toCppString());
      }
    }
  }
  return names;
}

bool canReuseUnitEmitters(const std::string& path) {
  RepoFile::init(path);
  SCOPE_EXIT { RepoFile::destroy(); };
  return RepoFile::globalData().hasRepoFileCapability(
    RepoFileCapability::RAW_UNIT_EMITTER_REUSE);
}

TEST(RepoFileTest, ReusableMetadataIsOptIn) {
  folly::test::TemporaryDirectory temp{"repo-file-reusable-opt-in"};
  auto const root = temp.path().native() + "/";
  auto const oldSourceRoot = std::exchange(Cfg::Server::SourceRoot, root);
  auto const oldRepoAuthoritative =
    std::exchange(Cfg::Repo::Authoritative, true);
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Server::SourceRoot = oldSourceRoot;
    Cfg::Repo::Authoritative = oldRepoAuthoritative;
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };

  auto const defaultPath = temp.path() / "default.hhbc";
  auto const reusablePath = temp.path() / "reusable.hhbc";
  std::vector<UnitSpec> const units{
    {root + "unit.php", "unit", "UNIT_CONSTANT", "unit_meth_caller"},
  };
  buildRepo(defaultPath.native(), units, 1);
  buildRepo(reusablePath.native(), units, 1, true);

  EXPECT_FALSE(canReuseUnitEmitters(defaultPath.native()));
  EXPECT_TRUE(canReuseUnitEmitters(reusablePath.native()));
  EXPECT_THAT(
    [&] {
      buildIncrementalRepo(
        (temp.path() / "from-default.hhbc").native(),
        defaultPath.native(),
        {},
        {},
        2
      );
    },
    ThrowsMessage<std::runtime_error>(
      HasSubstr("cannot be used as an incremental base"))
  );
  buildIncrementalRepo(
    (temp.path() / "from-reusable.hhbc").native(),
    reusablePath.native(),
    {},
    {},
    2
  );
}

TEST(RepoFileTest, IncrementalBuildReplacesAddsAndRemovesUnits) {
  folly::test::TemporaryDirectory temp{"repo-file-incremental"};
  auto const root = temp.path().native() + "/";
  auto const oldSourceRoot = std::exchange(Cfg::Server::SourceRoot, root);
  auto const oldRepoAuthoritative =
    std::exchange(Cfg::Repo::Authoritative, true);
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Server::SourceRoot = oldSourceRoot;
    Cfg::Repo::Authoritative = oldRepoAuthoritative;
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };

  auto const absolute = [&](const char* path) { return root + path; };
  auto const basePath = temp.path() / "base.hhbc";
  auto const outputPath = temp.path() / "incremental.hhbc";

  buildRepo(
    basePath.native(),
    {
      {
        absolute("removed/nested.php"),
        "removed",
        "REMOVED_CONSTANT",
        "shared_meth_caller"
      },
      {absolute("replaced.php"), "replaced-old", "REPLACED_OLD_CONSTANT"},
      {
        absolute("unchanged.php"),
        "unchanged",
        "UNCHANGED_CONSTANT",
        "shared_meth_caller",
        "surviving_meth_caller"
      },
    },
    1,
    true
  );
  buildIncrementalRepo(
    outputPath.native(),
    basePath.native(),
    {
      {absolute("replaced.php"), "replaced-new", "REPLACED_NEW_CONSTANT"},
      {
        absolute("added.php"),
        "added",
        "ADDED_CONSTANT",
        "surviving_meth_caller",
        "MethCaller$C$memoize_impl"
      },
    },
    {"removed"},
    2
  );

  RepoFile::init(outputPath.native());
  SCOPE_EXIT { RepoFile::destroy(); };
  RepoFile::loadGlobalTables(true);

  AutoloadHandler autoload;
  autoload.requestInit();
  SCOPE_EXIT { autoload.requestShutdown(); };

  EXPECT_EQ(RepoFile::globalData().Signature, 2);
  EXPECT_TRUE(RepoFile::globalData().hasRepoFileCapability(
    RepoFileCapability::RAW_UNIT_EMITTER_REUSE));
  EXPECT_EQ(RepoFile::numUnits(), 3);
  EXPECT_EQ(RepoFile::findUnitPath(0), makeStaticString(absolute("replaced.php")));
  EXPECT_EQ(RepoFile::findUnitPath(1), makeStaticString(absolute("added.php")));
  EXPECT_EQ(RepoFile::findUnitPath(2), makeStaticString(absolute("unchanged.php")));
  EXPECT_EQ(RepoFile::findUnitPath(3), nullptr);

  std::vector<std::string> paths;
  for (auto const path : RepoFile::enumerateUnits()) {
    paths.push_back(path->toCppString());
  }
  EXPECT_EQ(
    paths,
    (std::vector<std::string>{
      absolute("added.php"),
      absolute("replaced.php"),
      absolute("unchanged.php"),
    })
  );

  EXPECT_EQ(loadFatalMessage(absolute("unchanged.php")), "unchanged");
  EXPECT_EQ(loadFatalMessage(absolute("replaced.php")), "replaced-new");
  EXPECT_EQ(loadFatalMessage(absolute("added.php")), "added");
  EXPECT_EQ(
    RepoFile::loadUnitEmitter(
      makeStaticString(absolute("removed/nested.php")), nullptr, false),
    nullptr
  );

  EXPECT_EQ(
    loadConstantNames(absolute("unchanged.php")),
    (std::vector<std::string>{"UNCHANGED_CONSTANT"})
  );
  EXPECT_EQ(
    loadConstantNames(absolute("replaced.php")),
    (std::vector<std::string>{"REPLACED_NEW_CONSTANT"})
  );
  EXPECT_EQ(
    loadConstantNames(absolute("added.php")),
    (std::vector<std::string>{"ADDED_CONSTANT"})
  );
  EXPECT_EQ(
    RepoFile::findUnitSymbols(
      makeStaticString(absolute("removed/nested.php"))),
    nullptr
  );
  auto const methCallerFile = autoload.getAutoloadMap()->getFunctionFile(
    std::string_view{"shared_meth_caller"});
  ASSERT_TRUE(methCallerFile);
  EXPECT_EQ(methCallerFile->native(), absolute("unchanged.php"));

  auto const survivingMethCallerFile =
    autoload.getAutoloadMap()->getFunctionFile(
      std::string_view{"surviving_meth_caller"});
  ASSERT_TRUE(survivingMethCallerFile);
  EXPECT_EQ(
    survivingMethCallerFile->native(), absolute("added.php"));
  EXPECT_EQ(
    loadFunctionNames(absolute("added.php")),
    (std::vector<std::string>{"surviving_meth_caller"})
  );
  EXPECT_FALSE(autoload.getAutoloadMap()->getFunctionFile(
    std::string_view{"MethCaller$C$memoize_impl"}));

  auto const replacedConstantFile = autoload.getAutoloadMap()->getConstantFile(
    std::string_view{"REPLACED_NEW_CONSTANT"});
  ASSERT_TRUE(replacedConstantFile);
  EXPECT_EQ(replacedConstantFile->native(), absolute("replaced.php"));
  EXPECT_FALSE(autoload.getAutoloadMap()->getConstantFile(
    std::string_view{"REPLACED_OLD_CONSTANT"}));
}

TEST(RepoFileTest, IncrementalBuildPreservesOrdinaryFunctionOverMethCaller) {
  folly::test::TemporaryDirectory temp{"repo-file-incremental-functions"};
  auto const root = temp.path().native() + "/";
  auto const oldSourceRoot = std::exchange(Cfg::Server::SourceRoot, root);
  auto const oldRepoAuthoritative =
    std::exchange(Cfg::Repo::Authoritative, true);
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Server::SourceRoot = oldSourceRoot;
    Cfg::Repo::Authoritative = oldRepoAuthoritative;
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };

  auto const retainedOrdinaryBasePath =
    temp.path() / "retained-ordinary-base.hhbc";
  auto const retainedOrdinaryOutputPath =
    temp.path() / "retained-ordinary-output.hhbc";
  auto const retainedMethCallerBasePath =
    temp.path() / "retained-meth-caller-base.hhbc";
  auto const retainedMethCallerOutputPath =
    temp.path() / "retained-meth-caller-output.hhbc";
  auto const ordinaryPath = root + "ordinary.php";
  auto const methCallerPath = root + "meth-caller.php";
  buildRepo(
    retainedOrdinaryBasePath.native(),
    {
      {
        ordinaryPath,
        "ordinary",
        "ORDINARY_CONSTANT",
        "",
        "",
        "shared_function"
      },
    },
    1,
    true
  );
  buildIncrementalRepo(
    retainedOrdinaryOutputPath.native(),
    retainedOrdinaryBasePath.native(),
    {
      {
        methCallerPath,
        "meth-caller",
        "METH_CALLER_CONSTANT",
        "shared_function"
      },
    },
    {},
    2
  );
  buildRepo(
    retainedMethCallerBasePath.native(),
    {
      {
        methCallerPath,
        "meth-caller",
        "METH_CALLER_CONSTANT",
        "shared_function"
      },
    },
    3,
    true
  );
  buildIncrementalRepo(
    retainedMethCallerOutputPath.native(),
    retainedMethCallerBasePath.native(),
    {
      {
        ordinaryPath,
        "ordinary",
        "ORDINARY_CONSTANT",
        "",
        "",
        "shared_function"
      },
    },
    {},
    4
  );

  auto const verifyOrdinaryFunctionWins = [&](const std::string& path) {
    RepoFile::init(path);
    SCOPE_EXIT { RepoFile::destroy(); };
    RepoFile::loadGlobalTables(false);

    EXPECT_EQ(
      loadFunctionNames(ordinaryPath),
      (std::vector<std::string>{"shared_function"})
    );
    EXPECT_EQ(
      loadFunctionNames(methCallerPath),
      (std::vector<std::string>{})
    );
  };
  verifyOrdinaryFunctionWins(retainedOrdinaryOutputPath.native());
  verifyOrdinaryFunctionWins(retainedMethCallerOutputPath.native());
}

TEST(RepoFileTest, IncrementalBuildHandlesUnitsAddedOutOfSerialNumberOrder) {
  folly::test::TemporaryDirectory temp{"repo-file-incremental-unit-order"};
  auto const root = temp.path().native() + "/";
  auto const oldSourceRoot = std::exchange(Cfg::Server::SourceRoot, root);
  auto const oldRepoAuthoritative =
    std::exchange(Cfg::Repo::Authoritative, true);
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Server::SourceRoot = oldSourceRoot;
    Cfg::Repo::Authoritative = oldRepoAuthoritative;
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };

  auto const basePath = temp.path() / "unit-order-base.hhbc";
  auto const outputPath = temp.path() / "unit-order-output.hhbc";
  auto const ordinaryPath = root + "ordinary.php";
  auto const methCallerPath = root + "meth-caller.php";
  auto const addedPath = root + "added.php";
  buildRepo(
    basePath.native(),
    {
      {
        ordinaryPath,
        "ordinary",
        "ORDINARY_CONSTANT",
        "",
        "",
        "shared_function"
      },
    },
    1,
    true
  );
  // Serial numbers run opposite to the order the units are added in, so no
  // added unit sits at the index matching its serial number.
  buildIncrementalRepo(
    outputPath.native(),
    basePath.native(),
    {
      {
        methCallerPath,
        "meth-caller",
        "METH_CALLER_CONSTANT",
        "shared_function"
      },
      {addedPath, "added", "ADDED_CONSTANT"},
    },
    {},
    2,
    true
  );

  RepoFile::init(outputPath.native());
  SCOPE_EXIT { RepoFile::destroy(); };
  RepoFile::loadGlobalTables(false);

  EXPECT_EQ(RepoFile::numUnits(), 3);
  EXPECT_EQ(RepoFile::findUnitPath(0), makeStaticString(addedPath));
  EXPECT_EQ(RepoFile::findUnitPath(1), makeStaticString(methCallerPath));
  EXPECT_EQ(RepoFile::findUnitPath(2), makeStaticString(ordinaryPath));

  EXPECT_EQ(loadFatalMessage(addedPath), "added");
  EXPECT_EQ(loadFatalMessage(methCallerPath), "meth-caller");
  EXPECT_EQ(loadFatalMessage(ordinaryPath), "ordinary");
  EXPECT_EQ(
    loadConstantNames(addedPath),
    (std::vector<std::string>{"ADDED_CONSTANT"})
  );
  EXPECT_EQ(
    loadConstantNames(ordinaryPath),
    (std::vector<std::string>{"ORDINARY_CONSTANT"})
  );

  // Resolving the added meth caller by serial number rather than by position
  // is what lets the retained unit's ordinary function take precedence.
  EXPECT_EQ(
    loadFunctionNames(ordinaryPath),
    (std::vector<std::string>{"shared_function"})
  );
  EXPECT_EQ(loadFunctionNames(methCallerPath), (std::vector<std::string>{}));
}

TEST(RepoFileTest, IncrementalBuildRejectsPersistentSymbolCollisions) {
  folly::test::TemporaryDirectory temp{"repo-file-incremental-collisions"};
  auto const root = temp.path().native() + "/";
  auto const oldSourceRoot = std::exchange(Cfg::Server::SourceRoot, root);
  auto const oldRepoAuthoritative =
    std::exchange(Cfg::Repo::Authoritative, true);
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Server::SourceRoot = oldSourceRoot;
    Cfg::Repo::Authoritative = oldRepoAuthoritative;
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };

  enum class SymbolKind {
    Type,
    TypeAlias,
    Function,
    Constant,
    Module,
  };
  struct CollisionCase {
    const char* label;
    const char* diagnosticKind;
    const char* symbol;
    SymbolKind baseKind;
    SymbolKind addedKind;
  };
  CollisionCase const cases[] = {
    {"type", "type", "Foo", SymbolKind::Type, SymbolKind::Type},
    {
      "type-to-alias",
      "symbol",
      "Bar",
      SymbolKind::Type,
      SymbolKind::TypeAlias
    },
    {
      "alias-type",
      "symbol",
      "Baz",
      SymbolKind::TypeAlias,
      SymbolKind::Type
    },
    {
      "type-alias",
      "type alias",
      "Qux",
      SymbolKind::TypeAlias,
      SymbolKind::TypeAlias
    },
    {
      "constant",
      "constant",
      "COLLIDING_CONSTANT",
      SymbolKind::Constant,
      SymbolKind::Constant
    },
    {
      "function",
      "function",
      "colliding_function",
      SymbolKind::Function,
      SymbolKind::Function
    },
    {
      "module",
      "module",
      "colliding.module",
      SymbolKind::Module,
      SymbolKind::Module
    },
  };

  for (size_t i = 0; i < std::size(cases); ++i) {
    auto const& collision = cases[i];
    auto const makeUnit = [&](const std::string& path,
                              SymbolKind kind,
                              bool base) {
      UnitSpec unit;
      unit.path = path;
      unit.fatalMessage = collision.label;
      unit.constantName =
        (base ? "BASE_CONSTANT_" : "ADDED_CONSTANT_") +
        std::to_string(i);
      switch (kind) {
        case SymbolKind::Type:
          unit.typeName = collision.symbol;
          break;
        case SymbolKind::TypeAlias:
          unit.typeAliasName = collision.symbol;
          break;
        case SymbolKind::Function:
          unit.functionName = collision.symbol;
          break;
        case SymbolKind::Constant:
          unit.constantName = collision.symbol;
          break;
        case SymbolKind::Module:
          unit.moduleName = collision.symbol;
          break;
      }
      return unit;
    };

    auto const label = std::string{collision.label};
    auto const basePath = temp.path() / (label + "-base.hhbc");
    auto const outputPath = temp.path() / (label + "-output.hhbc");
    buildRepo(
      basePath.native(),
      {makeUnit(root + label + "-base.php", collision.baseKind, true)},
      i + 1,
      true
    );

    EXPECT_THAT(
      [&] {
        buildIncrementalRepo(
          outputPath.native(),
          basePath.native(),
          {
            makeUnit(
              root + label + "-added.php", collision.addedKind, false)
          },
          {},
          i + 2
        );
      },
      ThrowsMessage<std::runtime_error>(AllOf(
        HasSubstr(
          std::string{"More than one "} + collision.diagnosticKind +
          " with the name " + collision.symbol
        ),
        HasSubstr(label + "-base.php"),
        HasSubstr(label + "-added.php")
      ))
    );
  }
}

TEST(RepoFileTest, IncrementalBuildPromotesRetainedMethCallerRepeatedly) {
  folly::test::TemporaryDirectory temp{"repo-file-incremental-meth-callers"};
  auto const root = temp.path().native() + "/";
  auto const oldSourceRoot = std::exchange(Cfg::Server::SourceRoot, root);
  auto const oldRepoAuthoritative =
    std::exchange(Cfg::Repo::Authoritative, true);
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Server::SourceRoot = oldSourceRoot;
    Cfg::Repo::Authoritative = oldRepoAuthoritative;
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };

  auto const basePath = temp.path() / "base.hhbc";
  auto const outputPath = temp.path() / "incremental.hhbc";
  auto const secondOutputPath = temp.path() / "incremental-again.hhbc";
  buildRepo(
    basePath.native(),
    {
      {
        root + "removed.php",
        "removed",
        "REMOVED_CONSTANT",
        "shared_meth_caller"
      },
      {
        root + "retained-one.php",
        "retained-one",
        "RETAINED_ONE_CONSTANT",
        "shared_meth_caller"
      },
      {
        root + "retained-two.php",
        "retained-two",
        "RETAINED_TWO_CONSTANT",
        "shared_meth_caller"
      },
    },
    1,
    true
  );
  buildIncrementalRepo(
    outputPath.native(), basePath.native(), {}, {"removed.php"}, 2
  );
  buildIncrementalRepo(
    secondOutputPath.native(),
    outputPath.native(),
    {
      {
        root + "retained-two.php",
        "retained-two-updated",
        "RETAINED_TWO_UPDATED_CONSTANT",
        "shared_meth_caller"
      },
    },
    {},
    3
  );

  RepoFile::init(secondOutputPath.native());
  SCOPE_EXIT { RepoFile::destroy(); };
  RepoFile::loadGlobalTables(false);

  EXPECT_EQ(RepoFile::globalData().Signature, 3);
  EXPECT_TRUE(RepoFile::globalData().hasRepoFileCapability(
    RepoFileCapability::RAW_UNIT_EMITTER_REUSE));
  EXPECT_EQ(RepoFile::numUnits(), 2);
  EXPECT_EQ(
    RepoFile::findUnitPath(0),
    makeStaticString(root + "retained-two.php")
  );
  EXPECT_EQ(
    RepoFile::findUnitPath(1),
    makeStaticString(root + "retained-one.php")
  );
  EXPECT_EQ(
    loadFatalMessage(root + "retained-two.php"),
    "retained-two-updated"
  );

  EXPECT_EQ(
    loadFunctionNames(root + "retained-one.php"),
    (std::vector<std::string>{})
  );
  EXPECT_EQ(
    loadFunctionNames(root + "retained-two.php"),
    (std::vector<std::string>{"shared_meth_caller"})
  );
}

TEST(RepoFileTest, IncrementalBuildRejectsChangedPackageInfo) {
  folly::test::TemporaryDirectory temp{"repo-file-incremental-package-info"};
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };
  auto const basePath = temp.path() / "base.hhbc";
  auto const outputPath = temp.path() / "incremental.hhbc";

  buildRepo(basePath.string(), {}, 1, true);

  PackageInfo packageInfo;
  packageInfo.m_packages.emplace("changed", PackageInfo::Package{});
  std::vector<std::string> invalidatedPaths;
  EXPECT_THAT(
    [&] {
      RepoFileData base{basePath.string()};
      RepoFileBuilder builder(outputPath.string(), true);
      finishRepo(builder, {}, 2, &packageInfo, &base, invalidatedPaths);
    },
    ThrowsMessage<std::runtime_error>(
      HasSubstr("base PackageInfo does not match the current build"))
  );
}

TEST(RepoFileTest, IncrementalBuildAcceptsEquivalentPackageInfo) {
  folly::test::TemporaryDirectory temp{"repo-file-incremental-package-info"};
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };
  auto const basePath = temp.path() / "base.hhbc";
  auto const outputPath = temp.path() / "incremental.hhbc";

  PackageInfo packageInfo;
  PackageInfo::Package package;
  package.m_includes.emplace("zeta");
  package.m_includes.emplace("alpha");
  package.m_soft_includes.emplace("soft-zeta");
  package.m_soft_includes.emplace("soft-alpha");
  package.m_include_paths.emplace("zeta/path");
  package.m_include_paths.emplace("alpha/path");
  packageInfo.m_packages.emplace("example", std::move(package));

  PackageInfo::Deployment deployment;
  deployment.m_packages.emplace("zeta");
  deployment.m_packages.emplace("alpha");
  deployment.m_soft_packages.emplace("soft-zeta");
  deployment.m_soft_packages.emplace("soft-alpha");
  packageInfo.m_deployments.emplace("intern", std::move(deployment));

  {
    RepoFileBuilder builder{basePath.string(), true};
    finishRepo(builder, {}, 1, &packageInfo);
  }
  {
    RepoFileData base{basePath.string()};
    RepoFileBuilder builder{outputPath.string(), true};
    finishRepo(builder, {}, 2, &packageInfo, &base);
  }
}

TEST(RepoFileTest, IncrementalBuildRejectsIncompatibleBases) {
  folly::test::TemporaryDirectory temp{"repo-file-incremental-base"};
  auto const root = temp.path().native() + "/";
  auto const oldSourceRoot = std::exchange(Cfg::Server::SourceRoot, root);
  auto const oldRepoAuthoritative =
    std::exchange(Cfg::Repo::Authoritative, true);
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Server::SourceRoot = oldSourceRoot;
    Cfg::Repo::Authoritative = oldRepoAuthoritative;
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };

  std::vector<UnitSpec> const units{
    {root + "unit.php", "unit", "UNIT_CONSTANT"},
  };
  auto const hhbbcBase = temp.path() / "hhbbc-base.hhbc";
  auto const declBase = temp.path() / "decl-base.hhbc";

  Cfg::Eval::UseHHBBC = true;
  EXPECT_THAT(
    [&] {
      buildRepo(
        (temp.path() / "hhbbc-reusable.hhbc").native(), units, 1, true);
    },
    ThrowsMessage<std::runtime_error>(
      HasSubstr("cannot reuse raw unit emitters"))
  );
  buildRepo(hhbbcBase.native(), units, 1);
  Cfg::Eval::UseHHBBC = false;

  Cfg::Eval::EnableDecl = true;
  EXPECT_THAT(
    [&] {
      buildRepo(
        (temp.path() / "decl-reusable.hhbc").native(), units, 1, true);
    },
    ThrowsMessage<std::runtime_error>(
      HasSubstr("cannot reuse raw unit emitters"))
  );
  buildRepo(declBase.native(), units, 1);
  Cfg::Eval::EnableDecl = false;

  EXPECT_FALSE(canReuseUnitEmitters(hhbbcBase.native()));
  EXPECT_FALSE(canReuseUnitEmitters(declBase.native()));

  EXPECT_THAT(
    [&] {
      buildIncrementalRepo(
        (temp.path() / "from-hhbbc.hhbc").native(),
        hhbbcBase.native(),
        {},
        {},
        2
      );
    },
    ThrowsMessage<std::runtime_error>(
      HasSubstr("cannot be used as an incremental base"))
  );
  EXPECT_THAT(
    [&] {
      buildIncrementalRepo(
        (temp.path() / "from-decl.hhbc").native(),
        declBase.native(),
        {},
        {},
        2
      );
    },
    ThrowsMessage<std::runtime_error>(
      HasSubstr("cannot be used as an incremental base"))
  );
}

TEST(RepoFileTest, IncrementalBuildRejectsBaseAsOutput) {
  folly::test::TemporaryDirectory temp{"repo-file-incremental-output"};
  auto const root = temp.path().native() + "/";
  auto const oldSourceRoot = std::exchange(Cfg::Server::SourceRoot, root);
  auto const oldUseHHBBC = std::exchange(Cfg::Eval::UseHHBBC, false);
  auto const oldEnableDecl = std::exchange(Cfg::Eval::EnableDecl, false);
  SCOPE_EXIT {
    Cfg::Server::SourceRoot = oldSourceRoot;
    Cfg::Eval::UseHHBBC = oldUseHHBBC;
    Cfg::Eval::EnableDecl = oldEnableDecl;
  };

  auto const basePath = temp.path() / "base.hhbc";
  buildRepo(
    basePath.native(),
    {{root + "unit.php", "unit", "UNIT_CONSTANT"}},
    1,
    true
  );

  EXPECT_THAT(
    [&] {
      buildIncrementalRepo(
        basePath.native(), basePath.native(), {}, {}, 2
      );
    },
    ThrowsMessage<std::runtime_error>(HasSubstr("aliases its base"))
  );

  auto const linkedOutput = temp.path() / "linked-output.hhbc";
  std::filesystem::create_hard_link(basePath.native(), linkedOutput.native());
  EXPECT_THAT(
    [&] {
      buildIncrementalRepo(
        linkedOutput.native(), basePath.native(), {}, {}, 2
      );
    },
    ThrowsMessage<std::runtime_error>(HasSubstr("aliases its base"))
  );

  {
    RepoFile::init(basePath.native());
    SCOPE_EXIT { RepoFile::destroy(); };
    RepoFile::loadGlobalTables(false);
    EXPECT_EQ(RepoFile::globalData().Signature, 1);
    EXPECT_EQ(RepoFile::numUnits(), 1);
    EXPECT_EQ(loadFatalMessage(root + "unit.php"), "unit");
  }

  auto const outputWithLinkedPart = temp.path() / "linked-part.hhbc";
  auto const linkedPart = outputWithLinkedPart.native() + ".part";
  std::filesystem::create_hard_link(basePath.native(), linkedPart);
  EXPECT_THAT(
    [&] {
      buildIncrementalRepo(
        outputWithLinkedPart.native(), basePath.native(), {}, {}, 2
      );
    },
    ThrowsMessage<std::runtime_error>(HasSubstr("aliases its base"))
  );
}

}
}
