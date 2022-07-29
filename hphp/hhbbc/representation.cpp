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
#include "hphp/hhbbc/representation.h"

#include "hphp/hhbbc/wide-func.h"

namespace HPHP::HHBBC::php {

//////////////////////////////////////////////////////////////////////

// Can't form non-const refs to bitfields, so use this to serde them
// instead.
#define SD_BITFIELD(X)                      \
  if constexpr (SerDe::deserializing) {     \
    decltype(X) v;                          \
    sd(v);                                  \
    X = v;                                  \
  } else {                                  \
    sd(X);                                  \
  }

//////////////////////////////////////////////////////////////////////

template <typename SerDe> void SrcInfo::serde(SerDe& sd) {
  sd(loc)
    (docComment);
}

template <typename SerDe> void Block::serde(SerDe& sd) {
  sd(hhbcs)
    (exnNodeId)
    (fallthrough)
    (throwExit)
    (initializer);
}

template <typename SerDe> void CatchRegion::serde(SerDe& sd) {
  sd(catchEntry)
    (iterId);
}

template <typename SerDe> void ExnNode::serde(SerDe& sd) {
  sd(idx)
    (depth)
    (children)
    (parent)
    (region);
}

template <typename SerDe> void Param::serde(SerDe& sd) {
  sd(defaultValue)
    (dvEntryPoint)
    (typeConstraint)
    (userTypeConstraint)
    (upperBounds)
    (userAttributes)
    (phpCode)
    (builtinType);
  SD_BITFIELD(inout);
  SD_BITFIELD(readonly);
  SD_BITFIELD(isVariadic);
}

template <typename SerDe> void Local::serde(SerDe& sd) {
  sd(name);
  SD_BITFIELD(id);
  SD_BITFIELD(killed);
  SD_BITFIELD(nameId);
  SD_BITFIELD(unusedName);
}

template <typename SerDe> void Func::serde(SerDe& sd,
                                           Unit* parentUnit,
                                           Class* parentClass) {
  // This isn't true in general, but is right now for where we're
  // using the serializer.
  if constexpr (SerDe::deserializing) {
    unit = parentUnit;
    cls = parentClass;
    originalUnit = nullptr;
    originalClass = parentClass;
  } else {
    assertx(unit == parentUnit);
    assertx(cls == parentClass);
    assertx(!originalUnit);
    assertx(originalClass == parentClass);
  }

  sd(name)
    (idx)
    (srcInfo)
    (attrs)
    (params)
    (locals)
    (dvEntries)
    (numIters)
    (mainEntry)
    (returnUserType)
    (originalFilename)
    (returnUBs)
    (retTypeConstraint)
    (requiredCoeffects)
    (coeffectEscapes)
    (staticCoeffects)
    (coeffectRules)
    (userAttributes)
    (exnNodes)
    (isNative);

  SD_BITFIELD(isClosureBody);
  SD_BITFIELD(isAsync);
  SD_BITFIELD(isGenerator);
  SD_BITFIELD(isPairGenerator);
  SD_BITFIELD(isMemoizeWrapper);
  SD_BITFIELD(isMemoizeWrapperLSB);
  SD_BITFIELD(isMemoizeImpl);
  SD_BITFIELD(isReified);
  SD_BITFIELD(noContextSensitiveAnalysis);
  SD_BITFIELD(hasInOutArgs);
  SD_BITFIELD(sampleDynamicCalls);
  SD_BITFIELD(hasCreateCl);
  SD_BITFIELD(isReadonlyReturn);
  SD_BITFIELD(isReadonlyThis);
  SD_BITFIELD(hasParamsWithMultiUBs);
  SD_BITFIELD(hasReturnWithMultiUBs);

  if constexpr (SerDe::deserializing) {
    sd(WideFunc::mut(this).blocks());
  } else {
    sd(WideFunc::cns(this).blocks());
  }
}

template <typename SerDe> void Prop::serde(SerDe& sd) {
  sd(name)
    (attrs)
    (userAttributes)
    (docComment)
    (userType)
    (typeConstraint)
    (ubs)
    (val);
}

template <typename SerDe> void Const::serde(SerDe& sd, Class* parentClass) {
  // This isn't true in general, but is right now for where we're
  // using the serializer.
  if constexpr (SerDe::deserializing) {
    cls = parentClass;
  } else {
    assertx(cls == parentClass);
  }

  sd(name)
    (val)
    (coeffects)
    (resolvedTypeStructure)
    (kind);
  SD_BITFIELD(invariance);
  SD_BITFIELD(isAbstract);
  SD_BITFIELD(isFromTrait);
}

template <typename SerDe> void Class::serde(SerDe& sd, Unit* parentUnit) {
  // This isn't true in general, but is right now for where we're
  // using the serializer.
  if constexpr (SerDe::deserializing) {
    unit = parentUnit;
    closureContextCls = nullptr;
  } else {
    assertx(unit == parentUnit);
  }

  sd(name)
    (srcInfo)
    (attrs)
    (id)
    (parentName)
    // closureContextCls is deliberately skipped. It will be encoded
    // by the parent unit.
    (interfaceNames)
    (includedEnumNames)
    (usedTraitNames)
    (requirements)
    (properties)
    (constants, this)
    (userAttributes)
    (enumBaseTy)
    (methods, parentUnit, this);

  SD_BITFIELD(hasReifiedGenerics);
  SD_BITFIELD(hasConstProp);
  SD_BITFIELD(sampleDynamicConstruct);
}

template <typename SerDe> void Constant::serde(SerDe& sd) {
  sd(name)
    (val)
    (attrs);
}

template <typename SerDe> void Module::serde(SerDe& sd) {
  sd(name)
    (srcInfo)
    (attrs)
    (userAttributes);
}

template <typename SerDe> void TypeAlias::serde(SerDe& sd) {
  sd(srcInfo)
    (name)
    (value)
    (attrs)
    (type)
    (nullable)
    (userAttrs)
    (typeStructure)
    (resolvedTypeStructure);
}

template <typename SerDe> void FatalInfo::serde(SerDe& sd) {
  sd(fatalLoc)
    (fatalOp)
    (fatalMsg);
}

template <typename SerDe> void Unit::serde(SerDe& sd) {
  sd(filename)
    .nullable(fatalInfo)
    (funcs, this, nullptr)
    (classes, this)
    (typeAliases)
    (constants)
    (modules)
    (srcLocs)
    (metaData)
    (fileAttributes)
    (moduleName);

  // We deferred encoding closureContextCls in Class. This is because
  // its difficult to refer to another class until we've seen all of
  // them. Now that we've seen all the classes, handle
  // closureContextCls.
  if constexpr (SerDe::deserializing) {
    hphp_fast_map<int32_t, Class*> idToCls;
    idToCls.reserve(classes.size());
    for (auto& c : classes) {
      assertx(c->id != kInvalidId);
      auto const DEBUG_ONLY inserted =
        idToCls.emplace(c->id, c.get()).second;
      assertx(inserted);
    }

    for (auto& c : classes) {
      assertx(!c->closureContextCls);
      if (!is_closure(*c)) continue;

      int32_t id;
      sd(id);
      if (id == kInvalidId) continue;

      auto const it = idToCls.find(id);
      assertx(it != idToCls.end());

      c->closureContextCls = it->second;
    }
  } else {
    for (auto const& c : classes) {
      if (!is_closure(*c)) {
        assertx(!c->closureContextCls);
        continue;
      }

      if (c->closureContextCls) {
        assertx(c->closureContextCls->unit == this);
        assertx(c->closureContextCls->id != kInvalidId);
        sd(c->closureContextCls->id);
      } else {
        sd(kInvalidId);
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////

#undef SD_BITFIELD

//////////////////////////////////////////////////////////////////////

template void SrcInfo::serde(BlobEncoder&);
template void SrcInfo::serde(BlobDecoder&);

template void Block::serde(BlobEncoder&);
template void Block::serde(BlobDecoder&);

template void CatchRegion::serde(BlobEncoder&);
template void CatchRegion::serde(BlobDecoder&);

template void ExnNode::serde(BlobEncoder&);
template void ExnNode::serde(BlobDecoder&);

template void Param::serde(BlobEncoder&);
template void Param::serde(BlobDecoder&);

template void Local::serde(BlobEncoder&);
template void Local::serde(BlobDecoder&);

template void Func::serde(BlobEncoder&, Unit*, Class*);
template void Func::serde(BlobDecoder&, Unit*, Class*);

template void Prop::serde(BlobEncoder&);
template void Prop::serde(BlobDecoder&);

template void Const::serde(BlobEncoder&, Class*);
template void Const::serde(BlobDecoder&, Class*);

template void Class::serde(BlobEncoder&, Unit*);
template void Class::serde(BlobDecoder&, Unit*);

template void Constant::serde(BlobEncoder&);
template void Constant::serde(BlobDecoder&);

template void Module::serde(BlobEncoder&);
template void Module::serde(BlobDecoder&);

template void TypeAlias::serde(BlobEncoder&);
template void TypeAlias::serde(BlobDecoder&);

template void FatalInfo::serde(BlobEncoder&);
template void FatalInfo::serde(BlobDecoder&);

template void Unit::serde(BlobEncoder&);
template void Unit::serde(BlobDecoder&);

//////////////////////////////////////////////////////////////////////

}
