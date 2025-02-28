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
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/verify-types.h"

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/type-alias-emitter.h"
#include "hphp/util/match.h"
#include "hphp/util/process.h"

#include <boost/range/combine.hpp>

namespace HPHP{

namespace {

using PType = boost::variant<PreClass*, PreTypeAlias*>;

struct ArgTC {
  const Func* fn;
  size_t arg;

  std::string show() const {
    return folly::to<std::string>(
      "parameter ", arg, " of ", fn->fullName()->slice()
    );
  }
};

struct RetTC {
  const Func* fn;

  std::string show() const {
    return folly::to<std::string>("return value of ", fn->fullName()->slice());
  }
};

struct PropTC {
  const Class* cls;
  const StringData* prop;

  std::string show() const {
    return folly::to<std::string>(
      "property ", cls->name()->slice(), "::", prop->slice()
    );
  }
};

////////////////////////////////////////////////////////////////////////////////

Optional<PType> findPreType(const StringData* name, FactsStore* fs) {
  auto const p = fs->getTypeOrTypeAliasFile(StrNR{name});
  if (!p) return {};

  auto const d = lookupUnit(p->m_path.get(), g_context->getCwd().data(),
                            nullptr, nullptr, true, false, true);
  if (!d) return {};

  for (auto& pcls : d->preclasses()) {
    if (pcls->name() == name) return pcls.get();
  }

  for (auto& pty : d->typeAliases()) {
    if (pty.name == name) return &pty;
  }

  return {};
}

bool loadTypeSafe(PType initial, FactsStore* fs) {
  std::vector<PType> stack;
  hphp_fast_set<PType> visited;

  auto const name = [&] (PType p) {
    return match<const char*>(
      p,
      [&] (PreClass* pcls)    { return pcls->name()->data(); },
      [&] (PreTypeAlias* pty) { return pty->name->data(); }
    );
  };

  auto const def = [&] (PType p, bool hard = false) {
    return match<bool>(
      p,
      [&] (PreClass* pcls)    { return Class::def(pcls, hard) != nullptr; },
      [&] (PreTypeAlias* pty) { return TypeAlias::def(pty, hard) != nullptr; }
    );
  };

  auto const load = [&] (const StringData* p) {
    if (p->empty() || Class::lookup(p) || TypeAlias::lookup(p)) return;

    auto ty = findPreType(p, fs);
    if (ty && !visited.count(*ty)) {
      if (def(*ty)) visited.emplace(*ty);
      else stack.emplace_back(*ty);
    } else if (!ty) {
      Logger::FError("Could not find type: {}", p->data());
    }
  };

  if (def(initial)) return true;
  stack.emplace_back(initial);

  while (!stack.empty()) {
    auto p = stack.back();
    if (visited.emplace(p).second) {
      if (def(p)) {
        stack.pop_back();
        continue;
      }

      match<void>(
        p,
        [&] (PreClass* pc) {
          if (auto const parent = pc->parent())    load(parent);
          for (auto const i : pc->interfaces())    load(i);
          for (auto const e : pc->includedEnums()) load(e);
          for (auto const t : pc->usedTraits())    load(t);
          if (pc->enumBaseTy().isUnresolved()) {
            load(pc->enumBaseTy().typeName());
          }
        },
        [&] (PreTypeAlias* pta) {
          if (pta->value.isUnresolved()) {
            for (auto& tc : eachTypeConstraintInUnion(pta->value)) {
              if (tc.isUnresolved()) load(tc.typeName());
            }
          }
        }
      );
    } else {
      stack.pop_back();
      if (!def(p)) {
        try {
          def(p, true);
          Logger::FWarning("Undetected dependency for type {}", name(p));
        } catch (const Exception& ex) {
          Logger::FError("Failed to define type {}: {}", name(p), ex.what());
          if (stack.empty()) return false;
        }
      }
    }
  }

  return true;
}

bool loadTypeSafe(const StringData* ty, FactsStore* fs) {
  if (ty->empty() || Class::lookup(ty) || TypeAlias::lookup(ty)) return true;
  auto pty = findPreType(ty, fs);
  return pty ? loadTypeSafe(*pty, fs) : false;
}

Unit* loadPathSafe(const StringData* path, FactsStore* fs) {
  auto const u = lookupUnit(path, g_context->getCwd().data(), nullptr,
                            nullptr, true, false, true);

  if (!u) return nullptr;

  bool failed = false;
  for (auto const& pc : u->preclasses()) {
    if (!PreClassEmitter::IsAnonymousClassName(pc->name()->slice())) {
      failed |= !loadTypeSafe(pc.get(), fs);
    }
  }

  for (auto& pta : u->typeAliases()) {
    failed |= !loadTypeSafe(&pta, fs);
  }

  return !failed ? u : nullptr;
}

////////////////////////////////////////////////////////////////////////////////

template<class T>
bool checkConstraint(const T& ctx,
                     const TypeConstraint& hhbcTy,
                     const TypeConstraint& srcTy,
                     FactsStore* fs) {
  if (hhbcTy.isUnresolved()) return true;
  if (srcTy.isUnresolved()) {
    for (auto& tc : eachTypeConstraintInUnion(srcTy)) {
      if (tc.isUnresolved() && !loadTypeSafe(tc.typeName(), fs)) {
        Logger::FError("Unable to load type constraint for {}", ctx.show());
        return false;
      }
    }
  }

  try {
    auto const resTy = srcTy.resolvedWithAutoload();
    if (resTy == hhbcTy) return true;

    Logger::FError("Type constraint for {} does not match!\n"
                   "\tHHBC: {}\n\tsrc:  {}\n",
                   ctx.show(), hhbcTy.debugName(), resTy.debugName());
  } catch (Exception& ex) {
    Logger::FError("Error resolving type constraint for {} ({}): {}",
                   ctx.show(), srcTy.debugName(), ex.what());
  }

  return false;
}

template<class TCtx>
bool checkConstraints(const TCtx& ctx,
                      const TypeIntersectionConstraint& hhbcTys,
                      const TypeIntersectionConstraint& srcTys,
                      FactsStore* fs) {
  if (hhbcTys.size() == 0) return true;

  if (hhbcTys.size() != srcTys.size()) {
    Logger::FError("Type constraints for {} have mismatched length",
                   ctx.show());
    return false;
  }

  bool status = true;
  for (uint32_t i = 0; i < hhbcTys.size(); ++i) {
    if (!checkConstraint(ctx, hhbcTys.range().at(i), srcTys.range().at(i), fs)) status = false;
  }

  return status;
}

bool checkFuncConstraints(const FuncEmitter* hhbc,
                          const Func* src,
                          FactsStore* fs) {
  bool status = checkConstraints(
    RetTC{src},
    hhbc->retTypeConstraints,
    src->returnTypeConstraints(),
    fs
  );
  if (src->numParams() != hhbc->params.size()) {
    Logger::FError(
        "Parameter count mismatch for {}", src->fullName()->data()
    );
    return false;
  }

  for (uint32_t i = 0; i < src->numParams(); ++i) {
    ArgTC ctx{src, i};
    auto const& hhbcParam = hhbc->params[i];
    auto const& srcParam = src->params()[i];

    if (!checkConstraints(
          ctx,
          hhbcParam.typeConstraints,
          srcParam.typeConstraints,
          fs)) {
      status = false;
    }
  }
  return status;
}

template<class TClass>
bool checkMethodConstraints(const PreClassEmitter* hhbc,
                            const TClass* src,
                            FactsStore* fs) {
  bool status = true;
  for (auto const hhbcFunc : hhbc->methods()) {
    auto const srcFunc = src->lookupMethod(hhbcFunc->name);
    if (!srcFunc) {
      if (!Func::isSpecial(hhbcFunc->name)) {
        Logger::FError(
          "Could not find method {} on class {}",
          hhbcFunc->name->data(),
          src->name()->data()
        );
        status = false;
      }
      continue;
    }
    if (!checkFuncConstraints(hhbcFunc, srcFunc, fs)) status = false;
  }

  return status;
}

template<class TProp>
bool checkPropConstraints(const PropTC& ctx,
                          const PreClassEmitter::Prop& hhbcProp,
                          const TProp& srcProp,
                          FactsStore* fs) {
  return checkConstraints(
    ctx,
    hhbcProp.typeConstraints(),
    srcProp.typeConstraints,
    fs
  );
}

bool checkClassConstraints(const PreClassEmitter* hhbc,
                           const PreClass* src,
                           FactsStore* fs) {
  if (PreClassEmitter::IsAnonymousClassName(src->name()->slice())) {
    return checkMethodConstraints(hhbc, src, fs);
  }

  auto const srcCls = Class::lookup(src->name());
  if (!srcCls) {
    Logger::FError("Unable to load class {}", src->name()->data());
    return false;
  }

  bool status = checkMethodConstraints(hhbc, srcCls, fs);

  auto& propMap = hhbc->propMap();
  for (size_t i = 0; i < propMap.size(); ++i) {
    auto const& hhbcProp = propMap[i];

    // We can't check flattened properties without fully resolving the hhbc
    // PreClass into a Class
    if (hhbcProp.attrs() & AttrTrait) continue;

    auto const slot = hhbcProp.attrs() & AttrStatic
      ? srcCls->lookupSProp(hhbcProp.name())
      : srcCls->lookupDeclProp(hhbcProp.name());

    if (slot == kInvalidSlot) {
      Logger::FError("Could not find property {}::{}",
                     src->name()->data(), hhbcProp.name()->data());
      status = false;
      continue;
    }

    PropTC ctx{srcCls, hhbcProp.name()};
    if (hhbcProp.attrs() & AttrStatic) {
      auto const& srcProp = srcCls->staticProperties()[slot];
      if (!checkPropConstraints(ctx, hhbcProp, srcProp, fs)) status = false;
    } else {
      auto const& srcProp = srcCls->declProperties()[slot];
      if (!checkPropConstraints(ctx, hhbcProp, srcProp, fs)) status = false;
    }
  }

  return status;
}

////////////////////////////////////////////////////////////////////////////////

bool checkAliases(const UnitEmitter& hhbc) {
  bool status = true;

  for (auto const& tdef : hhbc.typeAliases()) {
    auto const ts = tdef->resolvedTypeStructure().get();
    if (!ts && tdef->value().isUnresolved()) continue;

    auto const srcDef = TypeAlias::lookup(tdef->name());
    if (!srcDef) {
      Logger::FError("Could not find TypeAlias {}!", tdef->name()->data());
      status = false;
      continue;
    }

    if (ts) {
      auto srcTS = srcDef->resolvedTypeStructureRaw();
      if (srcTS.isNull()) {
        bool persist = true;
        srcTS = TypeStructure::resolve(
          StrNR{srcDef->name()},
          srcDef->typeStructure(),
          persist
        );
      }
      if (srcTS.isNull()) {
        Logger::FError("TypeAlias {} cannot be resolved!",
                       tdef->name()->data());
        status = false;
      }
      if (ts && !srcTS.isNull() &&
          !tvEqual(make_tv<KindOfDict>(ts), make_tv<KindOfDict>(srcTS.get()))) {
        auto const hhbc = internal_serialize(VarNR{ts});
        auto const src = internal_serialize(srcTS);
        Logger::FError("TypeAlias {} has non-matching structure!\n"
                       "\tHHBC: {}\n\tsrc: {}",
                       tdef->name()->data(), hhbc.data(), src.data());
        status = false;
      }
    }

    if (!tdef->value().isUnresolved()) {
      if (!(srcDef->value == tdef->value())) {
        Logger::FError("TypeAlias {} has non-matching constraint!\n"
                       "\tHHBC: {}\n\tsrc:  {}\n",
                       tdef->name()->data(), tdef->value().debugName(),
                       srcDef->value.debugName());
        status = false;
      }
    }
  }

  return status;
}

bool checkTypeConstants(const UnitEmitter& hhbc) {
  bool status = true;

  for (auto const& cdef : hhbc.preclasses()) {
    if (PreClassEmitter::IsAnonymousClassName(cdef->name()->slice())) {
      continue;
    }

    auto const cls = Class::lookup(cdef->name());
    if (!cls) {
      Logger::FError("Unable to load class {}", cdef->name()->data());
      status = false;
      continue;
    }

    auto const& constMap = cdef->constMap();
    for (size_t idx = 0; idx < constMap.size(); ++idx) {
      auto& cconst = constMap[idx];
      if (auto const ts = cconst.resolvedTypeStructure().get()) {
        auto const cns = cls->clsCnsGet(cconst.name(),
                                        ConstModifiers::Kind::Type);
        if (!cns.is_init()) {
          Logger::FError("Unable to load type constant {}::{}",
                         cls->name()->data(), cconst.name()->data());
          status = false;
        } else if (!tvEqual(cns, make_tv<KindOfDict>(ts))) {
          auto const hhbc = internal_serialize(VarNR{ts});
          auto const src = internal_serialize(tvAsCVarRef(cns));
          Logger::FError("Type constant {}::{} has non-matching structure!\n"
                         "\tHHBC: {}\n\tsrc: {}",
                         cls->name()->data(), cconst.name()->data(),
                         hhbc.data(), src.data());
          status = false;
        }
      }
    }
  }

  return status;
}


bool checkTypeHints(const UnitEmitter& hhbc, Unit* src, FactsStore* fs) {
  hphp_fast_map<const StringData*, Func*> funcs;
  hphp_fast_map<const StringData*, PreClass*> classes;

  for (auto func : src->funcs()) funcs[func->name()] = func;
  for (auto& pcls : src->preclasses()) classes[pcls->name()] = pcls.get();

  bool status = true;

  for (auto& fe : hhbc.fevec()) {
    auto const srcFunc = folly::get_default(funcs, fe->name);
    if (!srcFunc) {
      Logger::FError("Unable to find function {}", fe->name->data());
      status = false;
      continue;
    }
    if (!checkFuncConstraints(fe.get(), srcFunc, fs)) status = false;
  }

  for (auto pce : hhbc.preclasses()) {
    auto const srcCls = folly::get_default(classes, pce->name());
    if (!srcCls) {
      if (!PreClassEmitter::IsAnonymousClassName(pce->name()->slice())) {
        Logger::FError("Unable to find class {}", pce->name()->data());
        status = false;
      }
      continue;
    }
    if (!checkClassConstraints(pce, srcCls, fs)) status = false;
  }

  return status;
}

bool checkResolved(const UnitEmitter& hhbc, Unit* src, FactsStore* fs) {
  bool status = true;

  status &= checkAliases(hhbc);
  status &= checkTypeHints(hhbc, src, fs);
  status &= checkTypeConstants(hhbc);

  return status;
}

////////////////////////////////////////////////////////////////////////////////
}

void compare_resolved_types(const std::string& hhbc_file,
                            const std::string& src_repo,
                            const std::vector<std::string>& paths) {

  if (src_repo.empty()) {
    fprintf(stderr, "No source repo specified\n");
    exit(HPHP_EXIT_FAILURE);
  }

  char resolved_repo[PATH_MAX];
  if (!realpath(src_repo.data(), resolved_repo)) {
    fprintf(stderr, "WARNING: unable to get realpath for src_repo: %s\n",
            src_repo.data());
    strncpy(resolved_repo, src_repo.data(), PATH_MAX);
  }

  hphp_process_init();
  SCOPE_EXIT {
    hphp_process_exit();
    Logger::FlushAll();
  };

  Cfg::Server::SourceRoot = resolved_repo;
  if (Cfg::Server::SourceRoot.back() != '/') Cfg::Server::SourceRoot += '/';

  RepoFile::init(hhbc_file);
  RepoFile::loadGlobalTables(false);

  SCOPE_EXIT { RepoFile::destroy(); };

  auto const units = [&] {
    if (!paths.empty()) {
      std::vector<const StringData*> ret;
      for (auto const& p : paths) {
        if (p.empty()) continue;
        if (p == ":system") {
          for (auto u : RepoFile::enumerateUnits()) {
            if (u->size() > 2 && u->data()[0] == '/' && u->data()[1] == ':') {
              ret.emplace_back(u);
            }
          }
          continue;
        }
        if (p.starts_with('/')) ret.emplace_back(makeStaticString(p));
        else ret.emplace_back(makeStaticString(Cfg::Server::SourceRoot + p));
      }
      if (!ret.empty()) return ret;
    }
    return RepoFile::enumerateUnits();
  }();
  auto const size = units.size();

  auto const nthreads = std::min<size_t>(size, Process::GetCPUCount());
  std::vector<std::thread> workers;
  workers.reserve(nthreads);

  std::atomic<size_t> index  = 0;
  std::atomic<bool>   failed = false;

  std::atomic<size_t> skipped   = 0;
  std::atomic<size_t> processed = 0;
  std::atomic<size_t> errored   = 0;
  std::atomic<size_t> bad       = 0;

  for (size_t i = 0; i < nthreads; ++i) {
    workers.emplace_back([&] {
      try {
        HphpSessionAndThread _{Treadmill::SessionKind::None};

        auto const opts = RepoOptions::forFile(src_repo + '/');
        g_context->onLoadWithOptions("", opts);

        auto const factory = FactsFactory::getInstance();
        auto const facts = factory ? factory->getForOptions(opts) : nullptr;
        if (!facts) {
          Logger::Error("Unable to initialize facts!");
          failed = true;
          return;
        }

        facts->ensureUpdated();

        while (true) {
          auto const i = index++;
          if (i >= size) break;

          if (i % 100000 == 0) {
            printf("Analyzed %.2f%% (%lu / %lu units)\n",
                   (double)i * 100 / size, i, size);
          }

          auto const uname = units[i];
          auto const hhbc = RepoFile::loadUnitEmitter(uname, nullptr, false);

          Unit* src = nullptr;
          try {
            src = hhbc->isASystemLib() ? SystemLib::findPersistentUnit(uname)
                                       : loadPathSafe(uname, facts);
            if (!src) {
              Logger::FVerbose("Skipping {}, not found", uname->data());
              skipped++;
              continue;
            }
          } catch (const Exception& ex) {
            Logger::FWarning("WARNING: Unable to load `{}': {}",
                             uname->data(), ex.what());
            errored++;
            continue;
          }

          processed++;

          try {
            if (checkResolved(*hhbc, src, facts)) continue;
            Logger::FError("Discovered errors processing {}", uname->data());
          } catch (const Exception& ex) {
            Logger::FError("Got exception while checking unit {}: {}",
                    uname->data(), ex.what());
          }
          failed = true;
          bad++;
        }
      } catch (const std::exception& e) {
        Logger::FError("Worker thread exited with exception: {}", e.what());
        failed = true;
      }
    });
  }

  for (auto& t : workers) t.join();

  fprintf(stderr,
          "Processed %lu units; Skipped %lu units; Encountered %lu load errors;"
          " Found %lu failing units\n",
          processed.load(), skipped.load(), errored.load(), bad.load());

  if (failed) {
    hphp_process_exit();
    Logger::FlushAll();

    fprintf(stderr, "Verification failed\n");
    exit(HPHP_EXIT_FAILURE);
  }
}

}
