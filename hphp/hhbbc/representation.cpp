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
                                           Class* parentClass) {
  if constexpr (SerDe::deserializing) {
    cls = parentClass;
  } else {
    assertx(cls == parentClass);
  }

  sd(name)
    (idx)
    (clsIdx)
    (srcInfo)
    (attrs)
    (params)
    (locals)
    (unit)
    (dvEntries)
    (numIters)
    (mainEntry)
    (returnUserType)
    (originalFilename)
    (originalUnit)
    (originalClass)
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

template <typename SerDe> void Const::serde(SerDe& sd) {
  sd(name)
    (cls)
    (val)
    (coeffects)
    (resolvedTypeStructure)
    (kind);
  SD_BITFIELD(invariance);
  SD_BITFIELD(isAbstract);
  SD_BITFIELD(isFromTrait);
}

template <typename SerDe> void Class::serde(SerDe& sd) {
  sd(name)
    (srcInfo)
    (attrs)
    (id)
    (unit)
    (parentName)
    (closureContextCls)
    (interfaceNames)
    (includedEnumNames)
    (usedTraitNames)
    (requirements)
    (properties)
    (constants)
    (userAttributes)
    (enumBaseTy)
    (methods, this);

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
    (funcs)
    (classes)
    (typeAliases)
    (constants)
    (modules)
    (srcLocs)
    (metaData)
    (fileAttributes)
    (moduleName);
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

template void Func::serde(BlobEncoder&, Class*);
template void Func::serde(BlobDecoder&, Class*);

template void Prop::serde(BlobEncoder&);
template void Prop::serde(BlobDecoder&);

template void Const::serde(BlobEncoder&);
template void Const::serde(BlobDecoder&);

template void Class::serde(BlobEncoder&);
template void Class::serde(BlobDecoder&);

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
