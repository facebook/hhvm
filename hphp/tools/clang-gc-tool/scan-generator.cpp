/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#undef __GXX_RTTI

#include "hphp/tools/clang-gc-tool/scan-generator.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <boost/filesystem.hpp>

namespace HPHP {

namespace fs = boost::filesystem;
using clang::ASTContext;
using clang::Decl;
using clang::NamedDecl;
using clang::FieldDecl;
using clang::CXXRecordDecl;
using clang::NamespaceDecl;
using clang::ClassTemplateDecl;
using clang::ClassTemplateSpecializationDecl;
using clang::VarDecl;
using clang::Type;
using clang::QualType;
using clang::PointerType;
using clang::ReferenceType;
using clang::ArrayType;
using clang::ConstantArrayType;
using clang::TemplateTypeParmType;
using clang::SubstTemplateTypeParmType;
using clang::TemplateSpecializationType;
using clang::SourceRange;
using clang::isa;
using clang::cast;
using clang::dyn_cast;
using clang::dyn_cast_or_null;
using clang::PrintingPolicy;
using clang::RawComment;
using clang::StringRef;
using clang::Rewriter;
using clang::TemplateTypeParmDecl;
using clang::NonTypeTemplateParmDecl;
using clang::TemplateTemplateParmDecl;
using clang::TemplateArgument;
using clang::CXXDestructorDecl;
using clang::CXXBaseSpecifier;
using clang::AS_protected;
using clang::AS_private;
using std::placeholders::_1;

bool ScanGenerator::isInDeclClass(const DeclSet& decls,
                                  const CXXRecordDecl* decl) const {
  decl = decl->getCanonicalDecl();
  return (exists(decls, decl) ||
          (isTemplate(decl) &&
           exists(decls,
                getTemplateDef(decl)->getTemplatedDecl()->getCanonicalDecl())));
}

bool ScanGenerator::isGcClass(const CXXRecordDecl* decl) const {
  return isInDeclClass(m_gcClasses, decl);
}

bool ScanGenerator::isGcClass(const Type& sty) const {
  const Type *typ = sty.getUnqualifiedDesugaredType();
  const Type& ty(*typ);
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return isGcClass(decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return isGcClass(stripType(ty));
  }
  return false;
}

bool ScanGenerator::hasScanMethod(const CXXRecordDecl* decl) const {
  auto hasScanFn = [this](const CXXRecordDecl* d) {
    return isTemplate(d) && this->hasScanMethod(d);
  };
  return (isInDeclClass(m_hasScanMethodSet, decl) ||
          isNestedInFn(decl, hasScanFn));
}

bool ScanGenerator::hasScanMethod(const Type& sty) const {
  const Type *typ = sty.getUnqualifiedDesugaredType();
  const Type& ty(*typ);
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return hasScanMethod(decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return hasScanMethod(stripType(ty));
  }
  return false;
}

bool ScanGenerator::isIgnored(const CXXRecordDecl* decl) const {
  auto isIgnoredFn = [this](const CXXRecordDecl* d) {
    return this->isIgnored(d);
  };
  return (decl->isLambda() ||
          isInDeclClass(m_ignoredClasses, decl) ||
          isNestedInFn(decl, isIgnoredFn));
}

bool ScanGenerator::isIgnored(const Type& ty) const {
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return isIgnored(decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return isIgnored(stripType(ty));
  } else if (auto tParam = dyn_cast<TemplateTypeParmType>(&ty)) {
    auto decl = dyn_cast<CXXRecordDecl>(tParam->getDecl()->getDeclContext());
    if (decl) return isIgnored(decl);
  } else if (auto tParam = dyn_cast<SubstTemplateTypeParmType>(&ty)) {
    return isIgnored(*tParam->getReplacedParameter());
  } else if (auto tParam = dyn_cast<TemplateSpecializationType>(&ty)) {
    auto tDecl = tParam->getTemplateName().getAsTemplateDecl();
    if (auto decl = dyn_cast<ClassTemplateDecl>(tDecl)) {
      return isIgnored(decl->getTemplatedDecl());
    }
  }
  const Type *styp = ty.getUnqualifiedDesugaredType();
  return styp != &ty ? isIgnored(*styp) : false;
}

bool ScanGenerator::isBadContainer(const CXXRecordDecl* decl) const {
  return isInDeclClass(m_badContainers, decl);
}

bool ScanGenerator::isBadContainer(const Type& sty) const {
  const Type *typ = sty.getUnqualifiedDesugaredType();
  const Type& ty(*typ);
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return isBadContainer(decl);
  } else if (auto tParam = dyn_cast<TemplateTypeParmType>(&ty)) {
    auto decl = dyn_cast<CXXRecordDecl>(tParam->getDecl()->getDeclContext());
    if (decl) return isBadContainer(decl);
  } else if (auto tParam = dyn_cast<SubstTemplateTypeParmType>(&ty)) {
    return isBadContainer(*tParam->getReplacedParameter());
  } else if (auto tParam = dyn_cast<TemplateSpecializationType>(&ty)) {
    auto tDecl = tParam->getTemplateName().getAsTemplateDecl();
    if (auto decl = dyn_cast<ClassTemplateDecl>(tDecl)) {
      return isBadContainer(decl->getTemplatedDecl());
    }
  }
  return false;
}

bool ScanGenerator::isClonedClass(const CXXRecordDecl* decl) const {
  return decl && isInDeclClass(m_privateDefs, decl);
}

bool ScanGenerator::isClonedClass(const Type& sty) const {
  const Type *typ = sty.getUnqualifiedDesugaredType();
  const Type& ty(*typ);
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return isClonedClass(decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return isClonedClass(stripType(ty));
  } else if (auto tParam = dyn_cast<TemplateTypeParmType>(&ty)) {
    auto decl = dyn_cast<CXXRecordDecl>(tParam->getDecl()->getDeclContext());
    if (decl) return isClonedClass(decl);
  } else if (auto tParam = dyn_cast<SubstTemplateTypeParmType>(&ty)) {
    return isClonedClass(*tParam->getReplacedParameter());
  } else if (auto tParam = dyn_cast<TemplateSpecializationType>(&ty)) {
    auto tDecl = tParam->getTemplateName().getAsTemplateDecl();
    if (auto decl = dyn_cast<ClassTemplateDecl>(tDecl)) {
      return isClonedClass(decl->getTemplatedDecl());
    }
  }
  return false;
}

bool ScanGenerator::isReqPtr(const CXXRecordDecl* decl) const {
  return isTemplate(decl) &&
    getTemplateClassName(decl) == "HPHP::req::ptr";
}

bool ScanGenerator::isReqPtr(const Type& sty) const {
  const Type *typ = sty.getUnqualifiedDesugaredType();
  const Type& ty(*typ);
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return isReqPtr(decl);
  } else if (auto tParam = dyn_cast<TemplateTypeParmType>(&ty)) {
    auto decl = dyn_cast<CXXRecordDecl>(tParam->getDecl()->getDeclContext());
    if (decl) return isReqPtr(decl);
  } else if (auto tParam = dyn_cast<SubstTemplateTypeParmType>(&ty)) {
    return isReqPtr(*tParam->getReplacedParameter());
  } else if (auto tParam = dyn_cast<TemplateSpecializationType>(&ty)) {
    auto tDecl = tParam->getTemplateName().getAsTemplateDecl();
    if (auto decl = dyn_cast<ClassTemplateDecl>(tDecl)) {
      return isReqPtr(decl->getTemplatedDecl());
    }
  }
  return false;
}

void ScanGenerator::generateWarnings(const CXXRecordDecl* decl) {
  decl = decl->getCanonicalDecl();
  if (!exists(m_checked, decl) &&
      !hasScanMethod(decl) &&
      !isIgnored(decl) &&
      needsScanMethod(decl) == NeedsScanFlag::Yes) {
    m_checked.insert(decl);

    if (isHiddenDecl(decl) && isAnonymous(decl)) {
      warning(decl,
              "no scan function generated for private anonymous class '%s'",
              getName(decl));
    }

    decl = decl->field_empty() ? decl->getDefinition() : decl;
    assert(decl);

    for (const auto& field : decl->fields()) {
      auto fieldType = field->getType();
      if (fieldType->isUnionType()) {
        warning(m_whys,
                field,
                "found union %s::%s",
                getName(decl),
                getName(field));
      } else if (fieldType->isVoidPointerType()) {
        warning(m_whys,
                field,
                "found void* %s::%s",
                getName(decl),
                getName(field));
      } else if (isPointerOrReferenceType(*fieldType) &&
                 needsScanMethod(getPointeeType(*fieldType)) ==
                 NeedsScanFlag::Yes) {
        warning(m_whys,
                field,
                "found raw pointer to %s (%s::%s)",
                getName(getPointeeType(*fieldType)),
                getName(decl),
                getName(field));
      } else if (isArrayType(*fieldType) &&
                 needsScanMethod(getElementType(*fieldType)) ==
                 NeedsScanFlag::Yes) {
        warning(m_whys,
                field,
                "found raw array of %s (%s::%s)",
                getName(getElementType(*fieldType)),
                getName(decl),
                getName(field));
      } else if (stripType(*fieldType).getAsCXXRecordDecl() &&
                 !getCanonicalDef(stripType(*fieldType).getAsCXXRecordDecl())) {
        warning(m_whys,
                field,
                "found opaque field type %s::%s",
                getName(decl),
                getName(field));
      }

      if (isBadContainer(*fieldType) &&
          needsScanMethod(*fieldType) == NeedsScanFlag::Yes) {
        warning(m_whys,
                field,
                "found use of bad container type (%s) %s::%s",
                getName(*fieldType),
                getName(decl),
                getName(field));

      }
    }
  }
}

ScanGenerator::NeedsScanFlag ScanGenerator::needsScanMethod(const Type& ty) {
  m_visited.clear();
  auto result = needsScanMethodImpl(ty);
  m_visited.clear();
  return result == NeedsScanFlag::Maybe ? NeedsScanFlag::No : result;
}

ScanGenerator::NeedsScanFlag
ScanGenerator::needsScanMethod(const CXXRecordDecl* decl) {
  m_visited.clear();
  auto result = needsScanMethodImpl(decl);
  m_visited.clear();
  return result == NeedsScanFlag::Maybe ? NeedsScanFlag::No : result;
}

// use visitor in here too.  make a closure of all types/decls that need gc
ScanGenerator::NeedsScanFlag
ScanGenerator::needsScanMethodImpl(const Type& sty) {
  const Type *typ = sty.getUnqualifiedDesugaredType();
  const Type& ty(*typ);
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return needsScanMethodImpl(decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return needsScanMethodImpl(stripType(ty));
  } else if (auto tParam = dyn_cast<TemplateTypeParmType>(&ty)) {
    if (tParam->getDecl()) {
      auto decl = dyn_cast<CXXRecordDecl>(tParam->getDecl()->getDeclContext());
      if (decl) return needsScanMethodImpl(decl);
    }
  } else if (auto tParam = dyn_cast<SubstTemplateTypeParmType>(&ty)) {
    return needsScanMethodImpl(*tParam->getReplacedParameter());
  } else if (auto tParam = dyn_cast<TemplateSpecializationType>(&ty)) {
    auto tDecl = tParam->getTemplateName().getAsTemplateDecl();
    if (auto decl = dyn_cast<ClassTemplateDecl>(tDecl)) {
      return needsScanMethodImpl(decl->getTemplatedDecl());
    }
  }
  return NeedsScanFlag::No;
}

ScanGenerator::NeedsScanFlag
ScanGenerator::needsScanMethodImpl(const CXXRecordDecl* decl) {
  decl = decl->getCanonicalDecl();

  if (isHiddenDecl(decl) && !isAnonymous(decl)) {
    m_privateDefs.insert(decl);
  }

  // TODO (t6956600): this can happen when a class is not fully defined.
  // Hopefully, the full definition will be available elsewhere.
  if (!getCanonicalDef(decl) ||
      (isTemplate(decl) &&
       !getCanonicalDef(getTemplateDef(decl)->getTemplatedDecl()))) {
    return NeedsScanFlag::No;
  }

  if (isGcClass(decl) || isInDeclClass(m_scanClasses, decl)) {
    return NeedsScanFlag::Yes;
  }

  if (isIgnored(decl)) {
    return NeedsScanFlag::No;
  }

  // If we are recursively visiting this class, return Maybe since
  // we may not have made a final decision yet.
  if (exists(m_visited, decl)) {
    return NeedsScanFlag::Maybe;
  }

  m_visited.insert(decl);

  auto result = NeedsScanFlag::Maybe;

  auto def = decl->getDefinition();
  assert(def);

  auto checkBaseClass = [&](const CXXBaseSpecifier& base) {
    auto sugaredBaseTy = base.getType();
    const auto baseTy = sugaredBaseTy->getUnqualifiedDesugaredType();
    if (auto baseClass = baseTy->getAsCXXRecordDecl()) {
      return needsScanMethodImpl(baseClass);
    } else if (auto tParam = dyn_cast<TemplateTypeParmType>(baseTy)) {
      return needsScanMethodImpl(
        cast<CXXRecordDecl>(tParam->getDecl()->getDeclContext()));
    } else if (auto tParam = dyn_cast<SubstTemplateTypeParmType>(baseTy)) {
      return needsScanMethodImpl(*tParam->getReplacedParameter());
    } else if (auto tParam = dyn_cast<TemplateSpecializationType>(baseTy)) {
      auto tDecl = tParam->getTemplateName().getAsTemplateDecl();
      if (auto decl = dyn_cast<ClassTemplateDecl>(tDecl)) {
        return needsScanMethodImpl(decl->getTemplatedDecl());
      }
      assert(isa<TemplateTemplateParmDecl>(tDecl));
    }
    return NeedsScanFlag::No;
  };

  bool sawSubclass = false;

  // Check base classes.
  for (const auto& base : def->bases()) {
    if (checkBaseClass(base) == NeedsScanFlag::Yes) {
      result = NeedsScanFlag::Yes;
      sawSubclass = true;
      break;
    }
  }

  // Check virtual base classes.
  for (const auto& base : def->vbases()) {
    if (checkBaseClass(base) == NeedsScanFlag::Yes) {
      result = NeedsScanFlag::Yes;
      sawSubclass = true;
      break;
    }
  }

  // Check all fields for scannable types.
  for (const auto& field : def->fields()) {
    auto needsScan = needsScanMethodImpl(*field->getType());

    if (needsScan == NeedsScanFlag::Yes) {
      if (result == NeedsScanFlag::Maybe) {
        auto fieldTypeDecl = stripType(*field->getType()).getAsCXXRecordDecl();

        if (fieldTypeDecl) {
          fieldTypeDecl = fieldTypeDecl->getCanonicalDecl();
          m_whys[field] = fieldTypeDecl;
        }

        // TODO (t6956600): not sure this warning is worthwhile.
        if (0 && !sawSubclass) {
          warning(m_whys,
                  field,
                  "'%s:%s' is not a subclass of any garbage collectible class",
                  getName(decl),
                  getName(field->getType()));
          assert(!fieldTypeDecl ||
                 isTemplate(fieldTypeDecl) ||
                 isGcClass(fieldTypeDecl) ||
                 hasScanMethod(fieldTypeDecl));
        }
      }
      if (m_verbose) {
        std::cout << getName(decl) << " field '" << getName(field)
                  << "' of type " << getName(field->getType())
                  << " needs scan\n";
      }
      result = NeedsScanFlag::Yes;
    }
  }

  if (result == NeedsScanFlag::No &&
      !isTemplate(decl) &&
      !isNestedInTemplate(decl) &&
      !hasScanMethod(decl)) {
    if (m_verbose) {
      std::cout << "Ignoring " << getName(decl) << "\n";
    }
    m_ignoredClasses.insert(decl->getCanonicalDecl());
  }

  // Total hack for things like unordered_map that hide use of scanable types.
  if (result == NeedsScanFlag::Maybe && isInDeclClass(m_gcContainers, decl)) {
    if (auto tdecl = dyn_cast<ClassTemplateSpecializationDecl>(decl)) {
      const auto& targs = tdecl->getTemplateArgs();
      for (unsigned i = 0; i < targs.size(); ++i) {
        if (targs[i].getKind() == TemplateArgument::Type) {
          auto needsScan = needsScanMethodImpl(*(targs[i].getAsType()));
          if (needsScan == NeedsScanFlag::Yes) {
            if (m_verbose) {
              std::cout << "gccontainer " << getName(decl)
                        << " needs scan because of "
                        << getName(*targs[i].getAsType()) << "\n";
            }
            result = NeedsScanFlag::Yes;
            break;
          }
        }
      }
    }
  }

  return result;
}

// Find if field decl is tagged with a HHVM_NEEDS_USER_SCAN_METHOD comment.
// Return empty StringRef if it is not tagged, otherwise the StringRef will
// point at the custom scan method text.
StringRef ScanGenerator::findUserMarkMethod(const FieldDecl* field) const {
  auto comment = findComment(field);
  if (comment) {
    auto text = comment->getRawText(m_context.getSourceManager());
    auto pos = text.find("HHVM_USER_SCAN_METHOD");
    if (pos != StringRef::npos) {
      auto start = pos + strlen("HHVM_USER_SCAN_METHOD") + 1;
      auto end = text.find(')', start);
      if (end != StringRef::npos) {
        return text.substr(start, end-start+1);
      }
    }
  }
  return StringRef();
}

const char* ScanGenerator::toStr(NeedsScanFlag flag) {
  switch(flag) {
    case NeedsScanFlag::No: return "No";
    case NeedsScanFlag::Yes: return "Yes";
    case NeedsScanFlag::Maybe: return "Maybe";
  }
  assert(0);
  return "";
}

bool ScanGenerator::visitCXXRecordDeclCommon(const CXXRecordDecl* udecl,
                                             bool forceScan) {
  auto decl = udecl->getCanonicalDecl();
  assert(decl == decl->getCanonicalDecl());

  // Only visit decls that are defined and a subclass of an existing
  // gcClass.
  if (decl->hasDefinition() && !exists(m_sawDefinition, decl)) {
    m_sawDefinition.insert(decl);

    if (isHiddenDecl(decl) && !isAnonymous(decl)) {
      m_privateDefs.insert(decl);
    }

    if (m_verbose) {
      std::cout << "Visiting " << getName(decl) << "\n";
    }
    if (needsScanMethod(decl) != NeedsScanFlag::No) {
      generateWarnings(decl);

      assert(!isIgnored(decl));

      if (!hasScanMethod(decl)) {
        if (m_verbose) {
          std::cout << getName(decl) << " needs scan.\n";
        }
        if (!isTemplate(decl)) {
          m_scanClasses.insert(decl->getCanonicalDecl());
        }
      } else if (m_verbose) {
        std::cout << getName(decl) << " has scan.\n";
      }
    } else {
      if (!isTemplate(decl) && !isNestedInTemplate(decl)) {
        if (m_verbose) {
          std::cout << "Ignoring " << getName(decl) << "\n";
        }
        m_ignoredClasses.insert(decl->getCanonicalDecl());
      }
    }
  }
  return true;
}

bool ScanGenerator::VisitFieldDecl(FieldDecl* decl) {
  if (auto record = decl->getType()->getAsCXXRecordDecl()) {
    visitCXXRecordDeclCommon(record);
  }
  return true;
}

bool ScanGenerator::VisitCXXRecordDecl(CXXRecordDecl* decl) {
  visitCXXRecordDeclCommon(decl);
  return true;
}

bool ScanGenerator::VisitClassTemplateDecl(ClassTemplateDecl* tdecl) {
  // Only visit decls that are defined and a subclass of an existing
  // gcClass.
  if (tdecl->isThisDeclarationADefinition()) {
    visitCXXRecordDeclCommon(tdecl->getTemplatedDecl());
  } else if (m_verbose) {
    std::cout << "Skipping template " << getName(tdecl) << "\n";
  }
  return true;
}

bool ScanGenerator::VisitClassTemplateSpecializationDecl(
  ClassTemplateSpecializationDecl* tdecl) {
  // Only visit decls that are defined and a subclass of an existing
  // gcClass.
  if (tdecl->isThisDeclarationADefinition()) {
    visitCXXRecordDeclCommon(tdecl);
  } else if (m_verbose) {
    std::cout << "Skipping template " << getName(tdecl) << "\n";
  }
  return true;
}

bool ScanGenerator::VisitVarDecl(VarDecl* vdecl) {
  auto ty = vdecl->getType();  // strip typedefs?
  if (auto cdecl = ty->getAsCXXRecordDecl()) {
    visitCXXRecordDeclCommon(cdecl);
  }
  return true;
}

bool ScanGenerator::VisitDeclRefExpr(clang::DeclRefExpr* declref) {
  if (getName(declref->getFoundDecl()) == "registerNativeDataInfo") {
    auto targs = declref->getTemplateArgs();
    for (unsigned i = 0; i < declref->getNumTemplateArgs(); ++i) {
      auto arg = targs[i].getTypeSourceInfo();
      auto argTy = arg->getType();
      auto decl = argTy->getAsCXXRecordDecl();

      assert(isa<CXXRecordDecl>(decl));

      decl = decl->getCanonicalDecl();

      if (!decl->hasDefinition()) return true;

      if (m_verbose) {
        std::cout << "Adding native data class = " << getName(decl) << "\n";
      }

      if (exists(m_sawDefinition, decl)) {
        m_sawDefinition.erase(m_sawDefinition.find(decl));
      }

      visitCXXRecordDeclCommon(decl, true);
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////

void ScanGenerator::dumpTemplateDecl(std::ostream& out,
                                     const char* first,
                                     const CXXRecordDecl* def) const {
  forEachTemplateParam(
    def,
    [&](const NamedDecl* param) {
      out << first;
      if (auto p = dyn_cast<TemplateTypeParmDecl>(param)) {
        out << (p->wasDeclaredWithTypename() ? "typename " : "class ")
            << getName(p);
        if (p->isParameterPack()) out << "...";
      } else if (auto p = dyn_cast<NonTypeTemplateParmDecl>(param)) {
        out << getName(p->getType()) << " " << getName(p);
      } else if (auto p = dyn_cast<TemplateTemplateParmDecl>(param)) {
        assert(!p->isParameterPack());
        const char* prefix = "template <";
        for (auto param : *(p->getTemplateParameters())) {
          out << prefix << "typename " << getName(param);
          prefix = ", ";
        }
        out << "> class " << getName(p);
      }
      first = ", ";
      return true;
    }
  );
  out << ">";
}

//
// Cloning methods.
//

void ScanGenerator::declareClass(std::ostream& out,
                                 const CXXRecordDecl* def,
                                 bool do_close,
                                 const std::string& skipNs) const {
  // parent namespaces
  auto namespaces = getParentNamespaces(def);

  for (auto ns : namespaces) {
    if (getName(ns) == skipNs) continue; // hacky, skip HPHP namespace
    out << "namespace " << getName(ns) << " {\n";
  }

  if (isTemplate(def)) {
    dumpTemplateDecl(out, "template <", def);
    out << " ";
    out << tagName(def) << " " << getTemplateClassName(def, true);
  } else {
    out << tagName(def) << " " << getName(def, true);
  }

  if (do_close) {
    out << ";\n";
    for (size_t i = 0; i < namespaces.size(); ++i) {
      if (getName(namespaces[i]) == skipNs) continue;
      out << "}\n";
    }
  }
}

void ScanGenerator::declareClonedClass(std::ostream& out,
                                       const CXXRecordDecl* def,
                                       bool do_close) const {
  out << "namespace HPHP {\n";
  out << "namespace Cloned {\n";
  declareClass(out, def, do_close, "HPHP");
  if (do_close) out << "}\n}\n";
}

std::string ScanGenerator::maybeCloneName(std::string str) const {
  auto pos = str.find("HPHP::");
  auto start = pos;
  if (pos != std::string::npos) pos += strlen("HPHP::");
  while (start < str.length()) {
    if (pos < str.length() &&
        (isalnum(str[pos]) || str[pos] == '_' || str[pos] == ':')) {
      ++pos;
    } else {
      auto candidate = str.substr(start, pos - start);
      auto itr = m_clonedNames.find(candidate);
      if (itr != m_clonedNames.end()) {
        auto rpos = str.rfind("::", pos);
        if (rpos != std::string::npos) {
          if (isNestedDecl(itr->second)) {
            str = str.replace(start, rpos - start, "Cloned");
          } else {
            str = str.replace(start, strlen("HPHP"), "Cloned");
          }
        } else {
          str = str.insert(start, "Cloned::");
        }
      }
      pos = str.find("HPHP::", pos);
      start = pos;
      if (pos != std::string::npos) pos += strlen("HPHP::");
    }
  }
  return str;
}

void
ScanGenerator::addScanDecls(DeclSet& decls, const CXXRecordDecl* decl) const {
  if (auto tdecl = dyn_cast<ClassTemplateSpecializationDecl>(decl)) {
    const auto& targs = tdecl->getTemplateArgs();
    for (unsigned i = 0; i < targs.size(); ++i) {
      if (targs[i].getKind() == TemplateArgument::Type) {
        addScanDecls(decls, *(targs[i].getAsType()));
      }
    }
  }
  decls.insert(decl->getCanonicalDecl());
}

void ScanGenerator::addScanDecls(DeclSet& decls, const Type& ty) const {
  if (auto decl = ty.getAsCXXRecordDecl()) {
    addScanDecls(decls, decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    addScanDecls(decls, stripType(ty));
  } else if (auto tParam = dyn_cast<TemplateTypeParmType>(&ty)) {
    auto decl = dyn_cast<CXXRecordDecl>(tParam->getDecl()->getDeclContext());
    if (decl) addScanDecls(decls, decl);
  } else if (auto tParam = dyn_cast<SubstTemplateTypeParmType>(&ty)) {
    addScanDecls(decls, *tParam->getReplacedParameter());
  } else if (auto tParam = dyn_cast<TemplateSpecializationType>(&ty)) {
    auto tDecl = tParam->getTemplateName().getAsTemplateDecl();
    if (auto decl = dyn_cast<ClassTemplateDecl>(tDecl)) {
      addScanDecls(decls, decl->getTemplatedDecl());
    }
 }
}

namespace {
std::string removeAnonymous(std::string str) {
  const char* anonymousStr = "(anonymous namespace)::";
  auto pos = str.find(anonymousStr);
  while (pos != std::string::npos) {
    str = str.erase(pos, strlen(anonymousStr));
    pos = str.find(anonymousStr);
  }
  return str;
}
std::string removeInitializer(std::string str) {
  auto pos = str.find('{');
  if (pos != std::string::npos) {
    str = str.erase(pos);
  }
  pos = str.find('{');
  if (pos != std::string::npos) {
    str = str.erase(pos);
  }
  return str;
}
}

bool ScanGenerator::cloneDefs(std::ostream& out,
                              std::ostream& header,
                              const CXXRecordDecl* decl) {
  bool res = false;
  decl = decl->getCanonicalDecl();
  if (decl->hasDefinition() && !exists(m_cloneVisited, decl)) {
    m_cloneVisited.insert(decl);
    decl = decl->getDefinition();
    for (const auto& base : decl->bases()) {
      auto baseType = base.getType()->getUnqualifiedDesugaredType();
      res |= cloneDefs(out, header, *baseType);
    }
    for (const auto& base : decl->vbases()) {
      auto baseType = base.getType()->getUnqualifiedDesugaredType();
      res |= cloneDefs(out, header, *baseType);
    }
    for (const auto& field : decl->fields()) {
      auto fieldType = field->getType()->getUnqualifiedDesugaredType();
      res |= cloneDefs(out, header, *fieldType);
    }
    if (isHiddenDecl(decl) &&
        !isAnonymous(decl) &&
        !exists(m_clonedNames, getName(decl))) {
      res |= cloneDef(out, header, decl);
    }
    // If decl is a gc container, then visit all template args.
    // TODO: fix this, load-elim.cpp, PhiKey
    if (isInDeclClass(m_gcContainers, decl)) {
      if (auto tdecl = dyn_cast<ClassTemplateSpecializationDecl>(decl)) {
        const auto& targs = tdecl->getTemplateArgs();
        for (unsigned i = 0; i < targs.size(); ++i) {
          if (targs[i].getKind() == TemplateArgument::Type) {
            res |= cloneDefs(out, header, *(targs[i].getAsType()));
          }
        }
      }
    }
  }
  return res;
}

bool ScanGenerator::cloneDefs(std::ostream& out,
                              std::ostream& header,
                              const Type& sty) {
  const Type *typ = sty.getUnqualifiedDesugaredType();
  const Type& ty(*typ);
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return cloneDefs(out, header, decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return cloneDefs(out, header, stripType(ty));
  } else if (auto tParam = dyn_cast<TemplateTypeParmType>(&ty)) {
    auto decl = dyn_cast<CXXRecordDecl>(tParam->getDecl()->getDeclContext());
    if (decl) return cloneDefs(out, header, decl);
  } else if (auto tParam = dyn_cast<SubstTemplateTypeParmType>(&ty)) {
    return cloneDefs(out, header, *tParam->getReplacedParameter());
  } else if (auto tParam = dyn_cast<TemplateSpecializationType>(&ty)) {
    auto tDecl = tParam->getTemplateName().getAsTemplateDecl();
    if (auto decl = dyn_cast<ClassTemplateDecl>(tDecl)) {
      return cloneDefs(out, header, decl->getTemplatedDecl());
    }
  }
  return false;
}

void ScanGenerator::cloneField(std::ostream& os,
                               const char* outfile,
                               FieldDecl* field) {
  auto fieldType = field->getType();
  auto ft = fieldType.getDesugaredType(m_context);

  field->setType(ft.getCanonicalType());

  // Make sure clones for fields are emitted first.
  addScanDecls(m_scanDecls[outfile], *field->getType());

  std::string str;
  llvm::raw_string_ostream ss(str);
  PrintingPolicy pp(m_context.getLangOpts());
  pp.SuppressScope = false;
  field->print(ss, pp);
  auto fieldStr = removeInitializer(removeAnonymous(ss.str()));
  os << "  " << maybeCloneName(fieldStr) << ";\n";
}

bool ScanGenerator::cloneFields(std::ostream& os,
                                std::ostream& header,
                                const char* outfile,
                                const CXXRecordDecl* def) {
  bool sawFieldScan = false;
  std::string suffix;
  for (const auto& field : def->fields()) {
    auto fieldType = field->getType();
    if (needsScanMethod(*fieldType) != NeedsScanFlag::No) {
      if (m_verbose) {
        warning(field, "%s cloned field needs scan", getName(field));
      }
      auto fieldTypeDecl = fieldType->getAsCXXRecordDecl();
      if (fieldTypeDecl && fieldTypeDecl->isUnion()) {
        auto name = isAnonymous(fieldTypeDecl) ? "skipAnonField" + suffix
          : getName(field);
        // TODO: assert?  this will fail in scan generation.
        if (m_verbose) {
          warning(field,
                  "Private class '%s' has union field needing scan",
                  getName(def));
        }
        os << "  /* unscanable union */\n";
        os << "  std::aligned_storage<"
           << m_context.getTypeSize(fieldType)/8
           << ", "
           << m_context.getTypeAlign(fieldType)/8
           << ">::type " << name << ";\n";
      } else {
        if (fieldTypeDecl && isAnonymous(fieldTypeDecl)) {
          // dump all members
          if (!getName(field).empty()) {
            os << "  struct {\n";
          }
          sawFieldScan |= cloneFields(os,
                                      header,
                                      outfile,
                                      getCanonicalDef(fieldTypeDecl));
          if (!getName(field).empty()) {
            os << "  } " << getName(field) << ";\n";
          }
        } else {
          cloneField(os, outfile, field);
        }
        sawFieldScan = true;
      }
    } else {
      auto name = isAnonymous(field) ? "skipAnonField" + suffix
        : getName(field);
      os << "  std::aligned_storage<"
         << m_context.getTypeSize(fieldType)/8
         << ", "
         << m_context.getTypeAlign(fieldType)/8
         << ">::type " << name << ";\n";
    }
    suffix += "_";
  }
  return sawFieldScan;
}

bool ScanGenerator::cloneDef(std::ostream& out,
                             std::ostream& header,
                             const CXXRecordDecl* def) {
  auto outfile = getDefinitionFilename(def);

  def = def->getCanonicalDecl();

  assert(def == def->getCanonicalDecl());
  assert(def->hasDefinition());

  if (needsScanMethod(def) != NeedsScanFlag::Yes) {
    if (m_verbose) {
      std::cout << "cloneDef: " << getName(def) << " needs no scan\n";
    }
    return false;
  }

  // TODO (t6956600): some templates and anonymous types are not handled
  // properly by cloneDef.

  if (exists(m_clonedNames, getName(def))) {
    if (m_verbose) {
      std::cout << "cloneDef: " << getName(def) << " already cloned\n";
    }
    return true;
  }

  m_clonedNames[getName(def)] = def;

  def = def->getDefinition();

  std::stringstream os;
  bool sawFieldScan = !isInAnonymousNamespace(def);

  declareClonedClass(os, def, false);

  const char* sep = " : ";
  // dump superclasses.
  for (const auto& base : def->bases()) {
    auto baseType = base.getType()->getUnqualifiedDesugaredType();
    os << sep << "public " << maybeCloneName(getName(*baseType));
    sep = ", ";
    addScanDecls(m_scanDecls[outfile], baseType->getAsCXXRecordDecl());
  }
  for (const auto& base : def->vbases()) {
    auto baseType = base.getType()->getUnqualifiedDesugaredType();
    os << sep << "public virtual " << maybeCloneName(getName(*baseType));
    sep = ", ";
    addScanDecls(m_scanDecls[outfile], baseType->getAsCXXRecordDecl());
  }

  os << " {\npublic:\n";
  // Hack to stub out any pure virtual methods from superclasses.
  for (auto itr = def->method_begin(); itr != def->method_end(); ++itr) {
    auto method = (*itr)->getCanonicalDecl();
    if (!isa<CXXDestructorDecl>(method) && method->isVirtual()) {
      std::string methodStr;
      llvm::raw_string_ostream ss(methodStr);
      method->print(ss);
      auto str = maybeCloneName(ss.str());
      auto pos = str.find('{');
      if (pos != std::string::npos) { str = str.substr(0,pos); }
      os << "  " << str << " { throw 0; }\n";
      // TODO (t6956600): addScanDecls for types in method?
    }
  }

  sawFieldScan |= cloneFields(os, header, outfile, def);

  // add dummy ctor to suppress warnings about reference fields.
  os << "  " << getName(def, true) << "();\n";
  os << "};\n";
  auto namespaces = getParentNamespaces(def);
  for (size_t i = 0; i < namespaces.size(); ++i) {
    if (getName(namespaces[i]) == "HPHP") continue;
    os << "}\n";
  }
  os << "}\n}\n";

  out << os.str();

  return sawFieldScan;
}

bool ScanGenerator::dumpFieldMarks(std::ostream& out,
                                   std::ostream& header,
                                   const CXXRecordDecl* def,
                                   bool isCloned,
                                   const std::string& prefix) {
  bool sawFieldScan = false;
  assert(def->hasDefinition() && def == def->getDefinition());

  if (m_verbose) {
    std::cout << "dumpFieldMarks for "
              << getName(def) << " (" << isCloned << ")\n";
  }

  const std::string indent = "  ";
  for (const auto& field : def->fields()) {
    QualType fieldType(field->getType());

    if (m_verbose) {
      std::cout << getName(def) << "::" << getName(field)
           << " (" << isCloned << ") needs scan = "
           << toStr(needsScanMethod(*fieldType)) << "\n"
           << getName(def) << "::" << getName(field)
           << " (" << isCloned << ") has scan = "
           << hasScanMethod(*fieldType) << "\n"
           << getName(def) << "::" << getName(field)
           << " (" << isCloned << ") ignored = "
           << isIgnored(*fieldType) << "\n";
    }

    // TODO (t6956600): make exception for opaque/void*, i.e. conservative scan?
    if (needsScanMethod(*fieldType) == NeedsScanFlag::Yes) {
      auto fieldName = prefix.empty()
        ? field->getNameAsString()
        : prefix + "." + field->getNameAsString();
      auto fieldTypeDecl = fieldType->getAsCXXRecordDecl();

      sawFieldScan = true;

      // For now, template classes will need to declare scan function as a
      // friend.
      if (isAnonymous(field) || // redundant?
          (fieldTypeDecl && isAnonymous(fieldTypeDecl)) ||
          fieldType->isUnionType()) {
        if (!fieldTypeDecl->isUnion()) {
          dumpFieldMarks(out,
                         header,
                         getCanonicalDef(fieldTypeDecl),
                         isCloned,
                         fieldName);
        } else {
          out << indent;
          // TODO (t6956600): static assert?  or just conservative scan?
          out << "assert(0); // unions '" << fieldName << "' not handled.\n";
        }
      } else {
        out << indent;

        std::string fieldStr = std::string("this_.") + fieldName;

        // if cloned, must cast back to original type for dispatching.
        if (fieldTypeDecl &&
            !isNestedDecl(fieldTypeDecl) &&
            isClonedClass(fieldTypeDecl)) {
          if (isInAnonymousNamespace(fieldTypeDecl)) {
            fieldStr = "reinterpret_cast<const "
              + maybeCloneName(getName(*fieldType))
              + "&>(" + fieldStr + ")";
          } else {
            fieldStr = "reinterpret_cast<const "
              + getName(*fieldType)
              + "&>(" + fieldStr + ")";
          }
        } else if (isPointerType(*fieldType) &&
                   isClonedClass(getPointeeType(*fieldType))) {
          auto castType =
            getPointeeType(*fieldType).getUnqualifiedDesugaredType();
          if (!isPointerType(*castType)) {
            fieldStr = "reinterpret_cast<const "
              + getName(castType->getAsCXXRecordDecl())
              + "*>(" + fieldStr + ")";
          } else {
            // TODO (t6956600): turn into static assert?
            out << "assert(0); // multi-level pointers '"
                << fieldName << "' not handled.\n";
          }
        }

        if (isArrayType(*fieldType)) {
          // TODO (t6956600): This case is not quite right.
          // Could be array over allocation trick, e.g. if size == 1.
          if (isa<ConstantArrayType>(*fieldType)) {
            const auto& arrType = cast<ConstantArrayType>(*fieldType);

            std::string nElemsStr;
            llvm::raw_string_ostream ss(nElemsStr);
            arrType.getSize().print(ss, false);

            if (hasScanMethod(*fieldType)) {
              out << "mark("
                  << "&" << fieldStr << "[0], "
                  << "&" << fieldStr << "[" << ss.str() << "]);\n";
            } else {
              if (isPointerType(getElementType(*fieldType))) {
                fieldStr = std::string("*") + fieldStr;
              }
              out << "for(size_t i = 0; i < " << ss.str() << "; ++i) { ";
              out << "scan(" << fieldStr << "[i], mark);";
              out << " }\n";
            }
          } else {
            // TODO (t6956600): Turn into static assert?
            out << "assert(0); // variable sized arrays '"
                << fieldName << "' not handled.\n";
          }
        // The check for req::ptr of a cloned field is necessary here since the
        // compiler can't see that the non-cloned class might be a subclass of
        // some other scanable type.  We need to extract the cloned pointer so
        // that it will dispatch to the proper scan method for the cloned type.
        } else if (isReqPtr(*fieldType)) {
          out << "if (this_." << fieldName << ") ";
          out << "mark(*" << fieldStr << ".get());\n";
        } else {
          if (isPointerType(*fieldType)) {
            fieldStr = std::string("*") + fieldStr;
            if (isPointerType(getPointeeType(*fieldType))) {
              // TODO (t6956600): this is hacky.  turn into static assert?
              out << "assert(0); // multi-level pointers '"
                  << fieldName << "' not handled.\n";
              continue;
            }
          }
          if (isPointerType(*fieldType)) {
            out << "if (this_." << fieldName << ") ";
          }
          if (hasScanMethod(*fieldType)) {
            out << "mark(" << fieldStr << ");\n";
          } else {
            out << "scan(" << fieldStr << ", mark);\n";
          }
        }
      }
    }
  }
  return sawFieldScan;
}

void ScanGenerator::dumpScanMethodDecl(std::ostream& out,
                                       const CXXRecordDecl* def,
                                       bool declareArgs,
                                       bool do_close,
                                       const std::string& instantiate,
                                       bool forceClone) const {
  if (!declareArgs || !isNestedDecl(def) || !instantiate.empty()) {
    bool isCloned = isClonedClass(def);

    if (declareArgs &&
        (isInAnonymousNamespace(def) ||
         (isAnonymous(def) && !isCloned))) {
      return;
    }

    assert((declareArgs && instantiate.empty()) || !declareArgs);

    if (declareArgs) {
      declareClass(out, def, true);
      out << "\n";
    }

    if (instantiate.empty()) {
      out << "namespace HPHP {\n";
      if (isTemplate(def)) {
        dumpTemplateDecl(out, "template <typename F_scan, ", def);
      } else {
        out << "template <typename F_scan>";
      }
    } else {
      if (isTemplate(def)) {
        return;  // no instantiation of template functions.
      } else {
        out << "namespace HPHP {\n";
        out << "template ";
      }
    }

    std::string className;
    if ((isNestedDecl(def) || isInAnonymousNamespace(def) || forceClone) &&
        isCloned) {
      className = getNsName(def);
      auto pos = className.find("::");
      if (pos != std::string::npos) {
        className = std::string("Cloned") + className.substr(pos);
      } else {
        className = std::string("Cloned::") + className;
      }
    } else {
      className = getName(def);
    }

    auto markType = instantiate.empty() ? "F_scan" : instantiate;
    auto instType = instantiate.empty() ? "" : "<" + instantiate + ">";

    out << " void scan"
        << instType
        << "(const " << className
        << "& this" << (isCloned ? "__" : "_") << ", "
        << markType << "& mark)";

    if (declareArgs || !instantiate.empty()) {
      out << ";\n";
    }

    if (do_close) {
      out << "}\n";
    }
  }
}

// Add scan method to class.
void ScanGenerator::addScanMethod(std::ostream& res,
                                  std::ostream& header,
                                  const CXXRecordDecl* def) {
  std::stringstream out;
  std::stringstream header_out;
  const std::string indent = "  ";

  assert(def->hasDefinition() && def->getDefinition() == def);

  bool isCloned = isClonedClass(def);

  if (isCloned) {
    m_cloneVisited.clear();
    isCloned = cloneDefs(out, header_out, def->getCanonicalDecl());
    isCloned = exists(m_clonedNames, getName(def));

    if (isCloned) {
      declareClonedClass(header_out, def, true);
      dumpScanMethodDecl(header_out, def, true, true, "", true);
    }
  }

  dumpScanMethodDecl(header_out, def, true);

  dumpScanMethodDecl(out, def, false, false);
  out << " {\n";
  if (isCloned) {
    assert(exists(m_clonedNames, getName(def)));
    auto cloneName = getNsName(def);
    auto pos = cloneName.find("HPHP::");
    if (pos != std::string::npos) {
      cloneName = std::string("Cloned::") +
        cloneName.substr(pos + strlen("HPHP::"));
    } else {
      cloneName = std::string("Cloned::") + cloneName;
    }
    out << "  const auto& this_ = reinterpret_cast<const "
        << cloneName << "&>(this__);\n";
    out << "  (void)this_;\n";
  }

  out << "#ifdef SCAN_DEBUG\n";
  out << "  std::cout << \"Scanning " << getName(def)
      << " @ \" << &this_ << \"\\n\";\n";
  out << "#endif\n";

  // Note: this assumes the sizing rules of clang/gcc are the same and that
  // these methods are generated and compiled by machines with the same
  // sizes.
  // This is currently not enabled because clang crashes on certain
  // template decls when calling getTypeSize().
  if (0 && !isTemplate(def)) {
    out << indent
        << "static_assert(sizeof(this_) == "
        << m_context.getTypeSize(def->getTypeForDecl())/8
        << ", \"Field added or removed from " << getName(def) << "\""
        << ");\n";
  }

  for (const auto& base : def->bases()) {
    if (needsScanMethod(*base.getType()) == NeedsScanFlag::Yes) {
      out << indent;
      if (isCloned) {
        out << "scan(reinterpret_cast<const " << getName(base.getType())
            << "&>(this_), mark);\n";
      } else {
        out << "scan(static_cast<const " << getName(base.getType())
            << "&>(this_), mark);\n";
      }
    }
  }

  for (const auto& base : def->vbases()) {
    if (needsScanMethod(*base.getType()) == NeedsScanFlag::Yes) {
      out << indent;
      if (isCloned) {
        out << "scan(reinterpret_cast<const " << getName(base.getType())
            << "&>(this_), mark);\n";
      } else {
        out << "scan(static_cast<const " << getName(base.getType())
            << "&>(this_), mark);\n";
      }
    }
  }

  dumpFieldMarks(out, header_out, def, isCloned);
  out << "}\n";
  out << "}\n";

  dumpScanMethodDecl(out, def, false, true, "IMarker");

  if (isCloned && !isInAnonymousNamespace(def) && !isNestedDecl(def)) {
    dumpScanMethodDecl(out, def, false, false, "", true);
    out << " {\n";
    out << "  scan(reinterpret_cast<const " << getName(def)
        << "&>(this__), mark);\n";
    out << "}\n";
    out << "}\n";
  }

  // Can't skip empty classes here because of template friend declaration issue.
  header << header_out.str();
  res << out.str() << "\n";
}

namespace {

struct LocationSorter {
  bool operator()(const CXXRecordDecl* a, const CXXRecordDecl* b) const {
    assert(a->hasDefinition() && b->hasDefinition());
    if (PluginUtil::isNestedDecl(a) && !PluginUtil::isNestedDecl(b)) {
      return true;
    } else {
      return a->getLocStart() < b->getLocStart();
    }
  }
};

template <typename P>
std::map<std::string, std::set<const CXXRecordDecl*, LocationSorter>>
partition(const P& pf, const DeclSet& s) {
  std::map<std::string, std::set<const CXXRecordDecl*, LocationSorter>> m;
  for(auto def : s) {
    m[pf(def)].insert(def);
  }
  return m;
}

}

void ScanGenerator::emitScanMethods() {
  auto partitionedScanClasses = partition(
    [this](const CXXRecordDecl* decl) {
      return this->getDefinitionFilename(decl);
    },
    m_scanClasses);

  for (const auto& part : partitionedScanClasses) {
    for (auto def : part.second) {
      std::stringstream scanStr, headerStr;

      // Skip and decls that have scan methods or are anonymous.
      // The string check here is a hack because clang doesn't return
      // the right answer for isAnonymousStructOrUnion for certain decls.
      auto cdef = cast<CXXRecordDecl>(def);

      if (hasScanMethod(cdef)) continue;

      assert(!isIgnored(cdef));

      if (isAnonymous(cdef)) {
        warning(cdef,
                "No scan for anonymous struct/union '%s'.",
                getName(cdef));
        continue;
      }

      assert(cdef->hasDefinition());
      addScanMethod(scanStr, headerStr, cdef->getDefinition());

      storeScanOutput(cdef, scanStr, headerStr);
    }
  }

  writeFiles();
}

void ScanGenerator::emitProlog(std::ostream& os, const DeclSet& decls) const {
  os << "// This file is auto generated.  Do not hand edit.\n";
  os << "// See hphp/tools/clang-gc-tool/README for details.\n";
  os << "// override-include-guard\n";

  std::set<std::string> headers;
  for (auto decl : decls) {
    auto filename = decl->hasDefinition()
      ? getDefinitionFilename(decl)
      : getFilename(decl);
    if (strlen(filename) > 2 && filename[0] == '.' && filename[1] == '/') {
      filename += 2;
    }

    if (isHeaderFile(filename)) {
      headers.insert(filename);
    }
  }

  for (const auto& header : headers) {
    os << "#include \"" << header << "\"\n";
  }

  os << "\n";
}

void ScanGenerator::emitEpilog(std::ostream& os) const {
}

void ScanGenerator::emitDeclProlog(std::ostream& os) const {
  os << "// This file is auto generated.  Do not hand edit.\n";
  os << "// See hphp/tools/clang-gc-tool/README for details.\n";
  os << "// override-include-guard\n\n";
}

void ScanGenerator::emitDeclEpilog(std::ostream& os) const {
}

namespace {
//
// Filename/directory utilities.
//

// Resolve any .. and . from a path.
fs::path resolve(const fs::path& p) {
  auto ap = fs::absolute(p);
  fs::path result;
  for (auto it=ap.begin(); it!=ap.end(); ++it) {
    if (*it == "..") {
      if (result.filename() == "..")
        result /= *it;
      else
        result = result.parent_path();
    } else if (*it != ".") {
      result /= *it;
    }
  }
  return result;
}

// Get dirname for a path.
std::string getDirname(const std::string& fullpath) {
  return resolve(fs::path(fullpath).remove_filename()).native();
}

// Get basename of file.
std::string stripExtension(const std::string& path) {
  auto sep = path.find_last_of('/');
  auto dot = path.find_last_of('.');
  std::string res;
  if (dot != std::string::npos && (dot > sep || sep == std::string::npos)) {
    res = path.substr(0, dot);
  } else {
    res = path;
  }
  if (res[0] == '.') res.erase(0,1);
  if (res[0] == '/') res.erase(0,1);
  return res;
}

// Essentially mkdir -p fullpath.
bool mkdirhier(const std::string& fullpath) {
  auto path = getDirname(fullpath);

  // Check first if the whole path exists
  if (access(path.c_str(), F_OK) >= 0) {
    return true;
  }

  char dir[PATH_MAX+1];
  snprintf(dir, sizeof(dir), "%s", path.c_str());

  for (char* p = dir + 1; *p; p++) {
    if (*p == '/') {
      *p = '\0';
      if (access(dir, F_OK) < 0) {
        if (mkdir(dir, 0777) < 0) {
          return errno == EEXIST;
        }
      }
      *p = '/';
    }
  }

  if (access(dir, F_OK) < 0) {
    if (mkdir(dir, 0777) < 0) {
      return errno == EEXIST;
    }
  }

  return true;
}

}

std::string ScanGenerator::getScanFilename(const std::string& file) const {
  return m_outdir + "/" + stripExtension(file) + "-scan";
}

void ScanGenerator::writeFiles() {
  std::map<std::string, std::stringstream> cattedScanMethods;
  std::map<std::string, std::stringstream> cattedScanHeaders;
  std::map<std::string, DeclSet> cattedScanDecls;

  assert(m_scanDecls.size() == m_scanMethods.size() &&
         m_scanMethods.size() == m_scanHeaders.size());

  for (auto& entry : m_scanMethods) {
    const auto outfile = getScanFilename(entry.first);
    cattedScanMethods[outfile] << entry.second.str();
  }

  for (auto& entry : m_scanHeaders) {
    const auto outfile = getScanFilename(entry.first);
    cattedScanHeaders[outfile] << entry.second.str();
  }

  for (auto& entry : m_scanDecls) {
    const auto outfile = getScanFilename(entry.first);
    cattedScanDecls[outfile].insert(entry.second.begin(),
                                    entry.second.end());
  }

  assert(cattedScanDecls.size() == cattedScanMethods.size() &&
         cattedScanMethods.size() == cattedScanHeaders.size());

  for (auto& entry : cattedScanMethods) {
    if (entry.second.str().empty()) continue;
    const auto outfile = entry.first;
    if (!mkdirhier(outfile)) {
      error("can't create directory %s", getDirname(outfile));
    }

    const auto scanfile = outfile + ".h";
    Lockfile lf(scanfile);
    // if we can't get a lock, the file has been written already.
    if (lf) {
      std::ofstream fs(scanfile, std::ofstream::out | std::ofstream::trunc);
      if (!fs) {
        lf.unlock();
        error("can't open %s", scanfile);
      }

      emitProlog(fs, cattedScanDecls[entry.first]);
      fs << entry.second.str();
      emitEpilog(fs);
    }

    const auto declfile = outfile + "-decl.h";
    Lockfile dlf(declfile);
    if (dlf) {
      std::ofstream dfs(declfile, std::ofstream::out | std::ofstream::trunc);
      if (!dfs) {
        dlf.unlock();
        error("can't open %s", declfile);
      }

      emitDeclProlog(dfs);
      auto str = cattedScanHeaders[entry.first].str();
      dfs << str;
      emitDeclEpilog(dfs);
    }
  }
}

void ScanGenerator::storeScanOutput(const CXXRecordDecl* decl,
                                    std::stringstream& scanners,
                                    std::stringstream& headers) {
  auto outfile = getDefinitionFilename(decl);
  m_scanMethods[outfile] << scanners.str();
  m_scanHeaders[outfile] << headers.str();
  addScanDecls(m_scanDecls[outfile], decl);
}

void ScanGenerator::preVisit() {
  // Collect all comments, currently unused.
  collectComments();

  // Initialize scanClasses with gcClasses.
  m_scanClasses = m_gcClasses;
  assert(std::includes(m_hasScanMethodSet.begin(),
                       m_hasScanMethodSet.end(),
                       m_gcClasses.begin(),
                       m_gcClasses.end()));
}

ScanGenerator::ScanGenerator(
  ASTContext& context,
  Rewriter& rewriter,
  DeclSet& hasScanMethod,
  DeclSet& ignoredClasses,
  const DeclSet& badContainers,
  const DeclSet& gcClasses,
  const DeclSet& gcContainers,
  const std::string& outdir,
  bool verbose
) : PluginUtil(context),
    m_rewriter(rewriter),
    m_hasScanMethodSet(hasScanMethod),
    m_ignoredClasses(ignoredClasses),
    m_badContainers(badContainers),
    m_gcClasses(gcClasses),
    m_gcContainers(gcContainers),
    m_outdir(outdir.empty() ? "." : outdir),
    m_verbose(verbose)
{ }

ScanGenerator::~ScanGenerator() { }

}
