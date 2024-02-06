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

#include "hphp/zend/zend-string.h"

namespace HPHP::HHBBC::php {

//////////////////////////////////////////////////////////////////////

FuncBytecode::Reuser* FuncBytecode::s_reuser = nullptr;

//////////////////////////////////////////////////////////////////////

template <typename SerDe> void SrcInfo::serde(SerDe& sd) {
  sd(loc)
    (docComment)
    ;
}

template <typename SerDe> void Block::serde(SerDe& sd) {
  sd(hhbcs)
    (exnNodeId)
    (fallthrough)
    (throwExit)
    (initializer)
    ;
}

template <typename SerDe> void CatchRegion::serde(SerDe& sd) {
  sd(catchEntry)
    (iterId)
    ;
}

template <typename SerDe> void ExnNode::serde(SerDe& sd) {
  sd(idx)
    (depth)
    (children)
    (parent)
    (region)
    ;
}

template <typename SerDe> void Param::serde(SerDe& sd) {
  sd(defaultValue)
    (dvEntryPoint)
    (typeConstraint)
    (userTypeConstraint)
    (upperBounds)
    (userAttributes)
    (phpCode)
    ;
  SERDE_BITFIELD(inout, sd);
  SERDE_BITFIELD(readonly, sd);
  SERDE_BITFIELD(isVariadic, sd);
}

template <typename SerDe> void Local::serde(SerDe& sd) {
  sd(name);
  SERDE_BITFIELD(id, sd);
  SERDE_BITFIELD(killed, sd);
  SERDE_BITFIELD(nameId, sd);
  SERDE_BITFIELD(unusedName, sd);
}

template <typename SerDe> void Func::serde(SerDe& sd, Class* parentClass) {
  ScopedStringDataIndexer _;

  if constexpr (SerDe::deserializing) {
    cls = parentClass;
  } else {
    assertx(cls == parentClass);
  }

  sd(name)
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
    (originalModuleName)
    (returnUBs)
    (retTypeConstraint)
    (requiredCoeffects)
    (coeffectEscapes)
    (staticCoeffects)
    (coeffectRules)
    (userAttributes)
    (exnNodes)
    (isNative)
    ;

  SERDE_BITFIELD(isClosureBody, sd);
  SERDE_BITFIELD(isAsync, sd);
  SERDE_BITFIELD(isGenerator, sd);
  SERDE_BITFIELD(isPairGenerator, sd);
  SERDE_BITFIELD(isMemoizeWrapper, sd);
  SERDE_BITFIELD(isMemoizeWrapperLSB, sd);
  SERDE_BITFIELD(isMemoizeImpl, sd);
  SERDE_BITFIELD(isReified, sd);
  SERDE_BITFIELD(noContextSensitiveAnalysis, sd);
  SERDE_BITFIELD(hasInOutArgs, sd);
  SERDE_BITFIELD(sampleDynamicCalls, sd);
  SERDE_BITFIELD(hasCreateCl, sd);
  SERDE_BITFIELD(isReadonlyReturn, sd);
  SERDE_BITFIELD(isReadonlyThis, sd);
  SERDE_BITFIELD(hasParamsWithMultiUBs, sd);
  SERDE_BITFIELD(hasReturnWithMultiUBs, sd);
  SERDE_BITFIELD(fromModuleLevelTrait, sd);
  SERDE_BITFIELD(requiresFromOriginalModule, sd);

  // Bytecode is not serialized automatically. If you want it to be
  // serialized, you must manually move it out of the Func and into a
  // FuncBytecode. To catch any situation where this is forgotten,
  // assert that the bytecode is always reset here (if for some reason
  // you don't want to serialize it at all, just reset it beforehand).
  assertx(!rawBlocks);
}

template <typename SerDe> void FuncBytecode::serde(SerDe& sd) {
  ScopedStringDataIndexer _;

  sd(name);

  if constexpr (SerDe::deserializing) {
    bc = CompressedBytecodePtr{
      WideFunc::compress(sd.template make<BlockVec>())
    };
    if (bc->empty()) {
      bc.reset();
    } else if (s_reuser) {
      // The above call to WideFunc deserialized the bytecode and
      // compressed it. This is always a new copy. If a reuser is
      // provided, we can try to find an existing identical bytecode
      // block and share that. Hash the compressed bytecode and look
      // it up in the table. We use SHA1, so we can use equality of
      // the hash as a proxy for equality of the bytecode.
      SHA1 sha1{string_sha1(
        folly::StringPiece{bc->data(), bc->size()}
      )};
      // Insert it. If one already exists, we'll get that back
      // instead. In either case, assign it back to this func.
      auto reused = s_reuser->emplace(sha1, std::move(bc)).first->second;
      bc = std::move(reused);
    }
  } else {
    sd(bc ? WideFunc::uncompress(*bc) : BlockVec{});
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
    (val)
    ;
}

template <typename SerDe> void Const::serde(SerDe& sd) {
  sd(name)
    (cls)
    (val)
    (coeffects)
    (resolvedTypeStructure)
    (kind)
    ;
  SERDE_BITFIELD(invariance, sd);
  SERDE_BITFIELD(isAbstract, sd);
  SERDE_BITFIELD(isFromTrait, sd);
}

template <typename SerDe> void Class::serde(SerDe& sd) {
  ScopedStringDataIndexer _;
  sd(name)
    (srcInfo)
    (attrs)
    (unit)
    (moduleName)
    (parentName)
    (closureContextCls)
    (closureDeclFunc)
    (interfaceNames)
    (includedEnumNames)
    (usedTraitNames)
    (requirements)
    (properties)
    (constants)
    (userAttributes)
    (enumBaseTy)
    (methods, this)
    ;

  SERDE_BITFIELD(hasReifiedGenerics, sd);
  SERDE_BITFIELD(hasConstProp, sd);
  SERDE_BITFIELD(sampleDynamicConstruct, sd);
}

template <typename SerDe> void ClassBytecode::serde(SerDe& sd) {
  ScopedStringDataIndexer _;
  sd(cls)
    (methodBCs)
    ;
}

template <typename SerDe> void Constant::serde(SerDe& sd) {
  sd(name)
    (val)
    (attrs)
    ;
}

template <typename SerDe> void Module::serde(SerDe& sd) {
  sd(name)
    (srcInfo)
    (attrs)
    (userAttributes)
    (exports)
    (imports)
    ;
}

template <typename SerDe> void TypeAlias::serde(SerDe& sd) {
  sd(srcInfo)
    (name)
    (attrs)
    (userAttrs)
    (value)
    (typeStructure)
    (resolvedTypeStructure)
    (kind)
    ;
}

template <typename SerDe> void FatalInfo::serde(SerDe& sd) {
  sd(fatalLoc)
    (fatalOp)
    (fatalMsg);
}

template <typename SerDe> void Unit::serde(SerDe& sd) {
  ScopedStringDataIndexer _;
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
    (moduleName)
    (extName)
    (packageInfo)
    ;
}

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

template void FuncBytecode::serde(BlobEncoder&);
template void FuncBytecode::serde(BlobDecoder&);

template void Prop::serde(BlobEncoder&);
template void Prop::serde(BlobDecoder&);

template void Const::serde(BlobEncoder&);
template void Const::serde(BlobDecoder&);

template void Class::serde(BlobEncoder&);
template void Class::serde(BlobDecoder&);

template void ClassBytecode::serde(BlobEncoder&);
template void ClassBytecode::serde(BlobDecoder&);

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
