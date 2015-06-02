/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
using clang::Type;
using clang::QualType;
using clang::PointerType;
using clang::ReferenceType;
using clang::ArrayType;
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
using clang::CXXDestructorDecl;
using clang::AS_protected;
using clang::AS_private;

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

bool ScanGenerator::isGcClass(const Type& ty) const {
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return isGcClass(decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return isGcClass(stripType(ty));
  }
  return false;
}

bool ScanGenerator::hasScanMethod(const CXXRecordDecl* decl) const {
  return isInDeclClass(m_hasScanMethodSet, decl);
}

bool ScanGenerator::hasScanMethod(const Type& ty) const {
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return hasScanMethod(decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return hasScanMethod(stripType(ty));
  }
  return false;
}

bool ScanGenerator::isBadContainer(const CXXRecordDecl* decl) const {
  return isInDeclClass(m_badContainers, decl);
}

bool ScanGenerator::isBadContainer(const Type& ty) const {
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return isBadContainer(decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return isBadContainer(stripType(ty));
  }
  return false;
}

void ScanGenerator::generateWarnings(const CXXRecordDecl* decl) {
  decl = decl->getCanonicalDecl();
  if (!exists(m_checked, decl) &&
      !hasScanMethod(decl) &&
      needsScanMethod(decl) == NeedsScanFlag::Yes) {
    m_checked.insert(decl);

    if (isHiddenDecl(decl)) {
      warning(decl, "private class %s needs scan", getName(decl));
    }

    for (auto itr = decl->field_begin(); itr != decl->field_end(); ++itr) {
      auto field = *itr;
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
      } else if(stripType(*fieldType).getAsCXXRecordDecl() &&
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

// use visitor in here too.  make a closure of all types/decls that need gc
ScanGenerator::NeedsScanFlag ScanGenerator::needsScanMethod(const Type& ty) {
  if (auto decl = ty.getAsCXXRecordDecl()) {
    return needsScanMethod(decl);
  } else if (isPointerOrReferenceType(ty) || isArrayType(ty)) {
    return needsScanMethod(stripType(ty));
  } else if (auto tParam = dyn_cast<TemplateTypeParmType>(&ty)) {
    auto decl = dyn_cast<CXXRecordDecl>(tParam->getDecl()->getDeclContext());
    if (decl) return needsScanMethod(decl);
  } else if (auto tParam = dyn_cast<SubstTemplateTypeParmType>(&ty)) {
    return needsScanMethod(*tParam->getReplacedParameter());
  } else if (auto tParam = dyn_cast<TemplateSpecializationType>(&ty)) {
    auto tDecl = tParam->getTemplateName().getAsTemplateDecl();
    if (auto decl = dyn_cast<ClassTemplateDecl>(tDecl)) {
      return needsScanMethod(decl->getTemplatedDecl());
    }
  }
  return NeedsScanFlag::No;
}

ScanGenerator::NeedsScanFlag
ScanGenerator::needsScanMethod(const CXXRecordDecl* decl) {
  // This can happen when a class is not fully defined.
  // Hopefully, the full definition will be available
  // elsewhere.
  if(!getCanonicalDef(decl)) {
    return NeedsScanFlag::No;
  }

  decl = decl->getCanonicalDecl();

  if (isGcClass(decl)) {
    return NeedsScanFlag::Yes;
  }

  if (hasScanMethod(decl)) {
    return NeedsScanFlag::No;
  }

  // If we are recursively visiting this class, return Maybe since
  // we may not have made a final decision yet.
  if (exists(m_visited, decl)) {
    return NeedsScanFlag::Maybe;
  }

  m_visited.insert(decl);

  auto result = NeedsScanFlag::Maybe;

  // If a class inherits from any class in gcClasses, mark it as needing
  // a scan method.
  bool sawSubclass = false;
  for (auto c : m_gcClasses) {
    if (decl->hasDefinition() && isSubclassOf(c, decl)) {
      m_whys[decl] = c;
      if (m_verbose) {
        std::cout << getName(decl) << " needs scan, is subclass of "
                  << getName(c) << "\n";
      }
      result = NeedsScanFlag::Yes;
      sawSubclass = true;
      break;
    }
  }

  // Check all fields for scannable types.
  for (auto itr = decl->field_begin(); itr != decl->field_end(); ++itr) {
    auto field = *itr;
    auto needsScan = needsScanMethod(*field->getType());

    if (needsScan == NeedsScanFlag::Yes && result == NeedsScanFlag::Maybe) {
      auto fieldTypeDecl = stripType(*field->getType()).getAsCXXRecordDecl();

      if (fieldTypeDecl) {
        fieldTypeDecl = fieldTypeDecl->getCanonicalDecl();
        m_whys[field] = fieldTypeDecl;
      }

      if (!sawSubclass) {
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
      if (m_verbose) {
        std::cout << getName(decl) << " field '" << getName(field)
             << "' of type " << getName(field->getType())
             << " needs scan\n";
      }
    }
    if(needsScan == NeedsScanFlag::Yes) {
      result = NeedsScanFlag::Yes;
    }
  }

  // Check base classes.
  for (auto base = decl->bases_begin(); base != decl->bases_end(); ++base) {
    NeedsScanFlag needsScan = NeedsScanFlag::No;
    // If this is not true, the base class is a template specialization.
    if (auto baseClass = base->getType()->getAsCXXRecordDecl()) {
      needsScan = needsScanMethod(baseClass);
    }
    if (needsScan == NeedsScanFlag::Yes) {
      result = needsScan;
    }
  }

  // Check virtual base classes.
  for (auto base = decl->vbases_begin(); base != decl->vbases_end(); ++base) {
    NeedsScanFlag needsScan = NeedsScanFlag::No;
    // If this is not true, the base class is a template specialization.
    if (auto baseClass = base->getType()->getAsCXXRecordDecl()) {
      needsScan = needsScanMethod(baseClass);
    }
    if (needsScan == NeedsScanFlag::Yes) {
      result = needsScan;
    }
  }

  // Update gcClasses and hasScanMethod sets based on the results of
  // the analysis.  For templates, we always make sure to store the
  // CXXRecordDecl corresponding to the template definition rather
  // than the ClassTemplateDecl.
  if (result == NeedsScanFlag::Yes) {
    if (isTemplate(decl)) {
      m_gcClasses.insert(
        getTemplateDef(decl)->getTemplatedDecl()->getCanonicalDecl());
    } else {
      m_gcClasses.insert(decl->getCanonicalDecl());
    }
  } else {
    result = NeedsScanFlag::No;
    if (isTemplate(decl)) {
      m_hasScanMethodSet.insert(
        getTemplateDef(decl)->getTemplatedDecl()->getCanonicalDecl());
    } else {
      m_hasScanMethodSet.insert(decl->getCanonicalDecl());
    }
  }

  assert(result == NeedsScanFlag::Yes || result == NeedsScanFlag::No);

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

void ScanGenerator::visitBaseClasses(const CXXRecordDecl* decl) {
  assert(decl->hasDefinition());
  for (auto base = decl->bases_begin(); base != decl->bases_end(); ++base) {
    auto baseClass = base->getType()->getAsCXXRecordDecl();
    if(!baseClass) continue;// XXXXXXXXXX hack (check elsewhere)
    assert(baseClass->hasDefinition());
    visitCXXRecordDeclCommon(baseClass);
  }

  for (auto base = decl->vbases_begin(); base != decl->vbases_end(); ++base) {
    auto baseClass = base->getType()->getAsCXXRecordDecl();
    if(!baseClass) continue;
    assert(baseClass->hasDefinition());
    visitCXXRecordDeclCommon(baseClass);
  }
}

bool ScanGenerator::visitCXXRecordDeclCommon(const CXXRecordDecl* udecl,
                                             bool forceScan) {
  auto decl = udecl->getCanonicalDecl();
  assert(decl == decl->getCanonicalDecl());

  // Only visit decls that are defined and a subclass of an existing
  // gcClass.
  if (decl->hasDefinition() && !exists(m_sawDefinition, decl)) {
    m_sawDefinition.insert(decl);

    if (isHiddenDecl(decl)) {
      m_privateDefs.insert(decl);
    }

    // visit superclasses first.
    visitBaseClasses(decl);

    if(forceScan || isSubclassOf(m_gcClasses, decl)) {
      if(m_verbose) {
        std::cout << "Visiting " << getName(decl) << "\n";
      }
      m_visited.clear();
      needsScanMethod(decl);
      m_visited.clear();
      generateWarnings(decl);
      m_visited.clear();
    } else {
      if(m_verbose) {
        std::cout << "Skipping " << getName(decl) << "\n";
      }
    }
  }
  return true;
}

bool ScanGenerator::VisitFieldDecl(FieldDecl* decl) {
  auto record = decl->getType()->getAsCXXRecordDecl();
  return !record || visitCXXRecordDeclCommon(record);
  return true;
}

bool ScanGenerator::VisitCXXRecordDecl(CXXRecordDecl* decl) {
  return visitCXXRecordDeclCommon(decl);
}

bool ScanGenerator::VisitClassTemplateDecl(ClassTemplateDecl* tdecl) {
  // Only visit decls that are defined and a subclass of an existing
  // gcClass.
  if (tdecl->isThisDeclarationADefinition()) {
    visitCXXRecordDeclCommon(tdecl->getTemplatedDecl());
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

      if(exists(m_sawDefinition, decl)) {
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
      } else if (auto p = dyn_cast<NonTypeTemplateParmDecl>(param)) {
        out << getName(p->getType()) << " " << getName(p);
      } else if (auto p = dyn_cast<TemplateTemplateParmDecl>(param)) {
        (void)p;
        assert(0); // NYI
      }
      first = ", ";
      return true;
    }
  );
  out << ">";
}

void ScanGenerator::dumpTemplateArgs(std::ostream& out,
                                     const char* first,
                                     const CXXRecordDecl* def) const {
  forEachTemplateParam(
    def,
    [&](const NamedDecl* param) {
      out << first << getName(param);
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
                                 bool do_close) const {
  // parent namespaces
  auto namespaces = getParentNamespaces(def);

  for (auto ns : namespaces) {
    if (getName(ns) == "HPHP") continue; // hacky, skip HPHP namespace
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
      if (getName(namespaces[i]) == "HPHP") continue;
      out << "}\n";
    }
  }
}

void ScanGenerator::declareClonedClass(std::ostream& out,
                                       const CXXRecordDecl* def,
                                       bool do_close) const {
  out << "namespace Cloned {\n";
  declareClass(out, def, do_close);
  if (do_close) out << "}\n";
}

std::string ScanGenerator::maybeCloneName(std::string str) const {
  auto pos = str.find("HPHP::");
  auto start = pos;
  if (pos != std::string::npos) pos += strlen("HPHP::");
  while (start < str.length()) {
    if (isalnum(str[pos]) || str[pos] == '_') {
      ++pos;
    } else {
      auto candidate = str.substr(start, pos);
      if (exists(m_clonedNames, candidate)) {
        str = str.replace(start, strlen("HPHP::"), "Cloned::");
      }
      pos = str.find("HPHP::", pos);
      start = pos;
      if (pos != std::string::npos) pos += strlen("HPHP::");
    }
  }
  return str;
}

bool ScanGenerator::cloneDef(std::ostream& out,
                             std::ostream& friends,
                             const CXXRecordDecl* def) {
  auto outfile = getDefinitionFilename(def);

  assert(def == def->getCanonicalDecl());
  assert(def->hasDefinition());

  if (needsScanMethod(def) != NeedsScanFlag::Yes) {
    //std::cout << "cloneDef, claim " << getName(def) << " needs no scan\n";
    return false;
  }

  if (exists(m_clonedNames, getName(def))) {  // XXX templates?
    //std::cout << "cloneDef, claim " << getName(def) << " already cloned\n";
    return true;
  }

  m_clonedNames.insert(getName(def));  // XXX templates?

  def = def->getDefinition();

  std::stringstream os;

  declareClonedClass(os, def, false);

  if (def->getNumBases() > 0 || def->getNumVBases() > 0) {
    const char* sep = " : ";
    // dump superclasses.
    for (auto base = def->bases_begin(); base != def->bases_end(); ++base) {
      auto baseClass = base->getType()->getAsCXXRecordDecl();
      os << sep << "public " << maybeCloneName(getName(base->getType()));
      sep = ", ";
      m_scanDecls[outfile].insert(baseClass);
    }
    for (auto base = def->vbases_begin(); base != def->vbases_end(); ++base) {
      auto baseClass = base->getType()->getAsCXXRecordDecl();
      os << sep << "public virtual "
          << maybeCloneName(getName(base->getType()));
      sep = ", ";
      m_scanDecls[outfile].insert(baseClass);
    }
  }
  os << " {\n";
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
      os << "  " << str << "{ throw 0; }\n";
    }
  }

  for (auto itr = def->field_begin(); itr != def->field_end(); ++itr) {
    auto field = *itr;
    auto fieldType = field->getType();
    if (needsScanMethod(*fieldType) == NeedsScanFlag::Yes) {
      if (m_verbose) {
        warning(field, "%s cloned field needs scan", getName(field));
      }
      std::string fieldStr;
      llvm::raw_string_ostream ss(fieldStr);
      PrintingPolicy pp(m_context.getLangOpts());
      pp.SuppressScope = true;
      // XXX this is a bit of a hack.
      field->setType(fieldType.getDesugaredType(m_context));
      field->print(ss, pp);
      auto str = ss.str();
      os << "  " << maybeCloneName(str) << ";\n";
      if (auto fdecl = fieldType->getAsCXXRecordDecl()) {
        // Make sure clones for fields are emitted first.
        if (exists(m_privateDefs, fdecl->getCanonicalDecl())) {
          cloneDef(out, friends, fdecl->getCanonicalDecl());
        } else {
          // XXXX deal with templates????
        }
        m_scanDecls[outfile].insert(fdecl->getCanonicalDecl());
      }
    } else {
      os << "  std::aligned_storage<"
         << m_context.getTypeSize(fieldType)/8
         << ", "
         << m_context.getTypeAlign(fieldType)/8
         << ">::type " << getName(field) << ";\n";
    }
  }
  os << "};\n";
  auto namespaces = getParentNamespaces(def);
  for (size_t i = 0; i < namespaces.size(); ++i) {
    if (getName(namespaces[i]) == "HPHP") continue;
    os << "}\n";
  }
  os << "}\n";

  out << os.str();

  return true;
}

bool ScanGenerator::isResourceOrObject(const CXXRecordDecl* def) const {
  return isSubclassOf(m_resourceDecl, def) || isSubclassOf(m_objectDecl, def);
}

void ScanGenerator::dumpFieldMarks(std::ostream& out,
                                   std::ostream& friends,
                                   const CXXRecordDecl* def,
                                   bool isCloned) {
  assert(def->hasDefinition() && def == def->getDefinition());

  if (m_verbose) {
    std::cout << "dumpFieldMarks for "
              << getName(def) << " (" << isCloned << ")\n";
  }

  const std::string indent = "  ";
  for (auto itr = def->field_begin(); itr != def->field_end(); ++itr) {
    auto field = *itr;

    if (m_verbose) {
      std::cout << getName(def) << "::" << getName(field)
           << " (" << isCloned << ") needs scan = "
           << toStr(needsScanMethod(*field->getType())) << "\n"
           << getName(def) << "::" << getName(field)
           << " (" << isCloned << ") has scan = "
           << hasScanMethod(*field->getType()) << "\n";
    }

    // TODO: This is overly persmissive....
    // make exception for opaque/void*?
    // See #6956600.
    if (!stripType(*field->getType()).isFundamentalType() &&
        (needsScanMethod(*field->getType()) == NeedsScanFlag::Yes ||
         hasScanMethod(*field->getType()))) {
      auto fieldName = field->getNameAsString();

      // For now, template classes will need to declare scan function as a
      // friend.
      if (field->isAnonymousStructOrUnion() ||
          field->getType()->isUnionType()) {
        auto anonField = field->getType()->getAsCXXRecordDecl();
        if (!anonField->isUnion()) {
          dumpFieldMarks(out, friends, getCanonicalDef(anonField), isCloned);
        } else {
          out << indent;
          out << "assert(0); // unions '" << fieldName << "' not handled.\n";
        }
      } else {
        out << indent;
        out << "mark(this_." << fieldName << ");\n";
      }
    }
  }
}

void ScanGenerator::dumpScanMethodDecl(std::ostream& out,
                                       const CXXRecordDecl* def,
                                       bool declareArgs) const {
  if (!declareArgs || !isNestedDecl(def)) {
    bool isCloned = exists(m_privateDefs, def->getCanonicalDecl());

    if (declareArgs) {
      declareClass(out, def, true);
      out << "\n";
    }

    if (isTemplate(def)) {
      dumpTemplateDecl(out, "template <typename F, ", def);
    } else {
      out << "template <typename F>";
    }

    std::string className;
    if (isNestedDecl(def) && isCloned) {
      className = "Cloned::" + getName(def, true);
    } else {
      className = getName(def, false);
    }

    out << " void scan(const "
        << className
        << "& this" << (isCloned ? "__" : "_")
        << ", F& mark)";

    if (declareArgs) out << ";\n\n";
  }
}

// Add scan method to class.
void ScanGenerator::addScanMethod(std::ostream& res,
                                  std::ostream& friends,
                                  const CXXRecordDecl* def) {
  std::stringstream out;
  const std::string indent = "  ";

  assert(def->hasDefinition() && def->getDefinition() == def);

  if (0 && def->field_empty()) { // can't do this.....?
    //assert(hasScanMethod(def));
    return;
  }

  bool isCloned = exists(m_privateDefs, def->getCanonicalDecl());

  if (isCloned) {
    isCloned = cloneDef(out, friends, def->getCanonicalDecl());
  }

  dumpScanMethodDecl(friends, def, true);

  dumpScanMethodDecl(out, def);
  out << " {\n";
  if (isCloned) {
    out << "  const auto& this_ = static_cast<const Cloned::"
        << getName(def, true) << "&>(this__);\n";
  }

  // Note: this assumes the sizing rules of clang/gcc are the same and that
  // these methods are generated and compiled by machines with the same
  // sizes.
  if (0 && !isTemplate(def)) {
    out << indent
        << "static_assert(sizeof(this_) == "
        << m_context.getTypeSize(def->getTypeForDecl())/8
        << ", \"Field added or removed from " << getName(def) << "\""
        << ");\n";
  }

  for (auto base = def->bases_begin(); base != def->bases_end(); ++base) {
    if (isGcClass(*base->getType())) {
      out << indent;
      out << "scan(static_cast<const " << getName(base->getType())
          << "&>(this_), mark);\n";
    }
  }

  for (auto base = def->vbases_begin(); base != def->vbases_end(); ++base) {
    if (isGcClass(*base->getType())) {
      out << indent;
      out << "scan(static_cast<const " << getName(base->getType())
          << "&>(this_), mark);\n";
    }
  }

  dumpFieldMarks(out, friends, def, isCloned);

  out << "}\n";

  res << out.str() << "\n";
}

void ScanGenerator::emitScanMethods() {
  // XXX sort by file/location
  for (auto def : m_gcClasses) {
    std::stringstream scanStr, friendStr;

    // Skip and decls that have scan methods or are anonymous.
    // The string check here is a hack because clang doesn't return
    // the right answer for isAnonymousStructOrUnion for certain decls.
    auto cdef = cast<CXXRecordDecl>(def);

    if(hasScanMethod(cdef)) continue;

    if (cdef->isAnonymousStructOrUnion() ||
        getName(cdef).find('.') != std::string::npos) {
      warning(cdef, "No scan for anonymous struct/union '%s'.", getName(cdef));
      continue;
    }

    assert(cdef->hasDefinition());
    addScanMethod(scanStr, friendStr, cdef->getDefinition());

    storeScanOutput(cdef, scanStr, friendStr);
  }

  writeFiles();
}

void ScanGenerator::emitProlog(std::ostream& os, const DeclSet& decls) const {
  os << "// This file is auto generated.  Do not hand edit.\n";
  os << "// See hphp/tools/clang-gc-tool/README for details.\n";
  os << "// override-include-guard\n";

  std::set<std::string> headers;
  for (auto decl : decls) { // XXXXXX sort here.
    auto filename = getDefinitionFilename(decl);
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
  os << "namespace HPHP {\n\n";
}

void ScanGenerator::emitEpilog(std::ostream& os) const {
  os << "\n}\n";
}

void ScanGenerator::emitDeclProlog(std::ostream& os) const {
  os << "// This file is auto generated.  Do not hand edit.\n";
  os << "// See hphp/tools/clang-gc-tool/README for details.\n";
  os << "// override-include-guard\n\n";
  os << "namespace HPHP {\n\n";
}

void ScanGenerator::emitDeclEpilog(std::ostream& os) const {
  os << "\n}\n";
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
  std::map<std::string, std::stringstream> cattedScanFriends;
  std::map<std::string, DeclSet> cattedScanDecls;

  assert(m_scanDecls.size() == m_scanMethods.size() &&
         m_scanMethods.size() == m_scanFriends.size());

  for (auto& entry : m_scanMethods) {
    const auto outfile = getScanFilename(entry.first);
    cattedScanMethods[outfile] << entry.second.str();
  }

  for (auto& entry : m_scanFriends) {
    const auto outfile = getScanFilename(entry.first);
    cattedScanFriends[outfile] << entry.second.str();
  }

  for (auto& entry : m_scanDecls) {
    const auto outfile = getScanFilename(entry.first);
    cattedScanDecls[outfile].insert(entry.second.begin(),
                                    entry.second.end());
  }

  assert(cattedScanDecls.size() == cattedScanMethods.size() &&
         cattedScanMethods.size() == cattedScanFriends.size());

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
      if(!dfs) {
        dlf.unlock();
        error("can't open %s", declfile);
      }

      emitDeclProlog(dfs);
      auto str = cattedScanFriends[entry.first].str();
      dfs << str;
      emitDeclEpilog(dfs);
    }
  }
}

void ScanGenerator::storeScanOutput(const NamedDecl* decl,
                                    std::stringstream& scanners,
                                    std::stringstream& friends) {
  auto outfile = getDefinitionFilename(decl);
  m_scanMethods[outfile] << scanners.str();
  m_scanFriends[outfile] << friends.str();
  m_scanDecls[outfile].insert(decl);
  //std::cout << "outfile for " << getName(decl) << " = " << outfile << "\n";
}

void ScanGenerator::preVisit() {
  // Collect all comments, currently unused.
  collectComments();

  for (auto decl : m_gcClasses) {
    if (getName(decl) == "HPHP::ResourceData") {
      m_resourceDecl = decl;
    }
    if (getName(decl) == "HPHP::ObjectData") {
      m_objectDecl = decl;
    }
  }
}

ScanGenerator::ScanGenerator(
  ASTContext& context,
  Rewriter& rewriter,
  DeclSet& hasScanMethod,
  const DeclSet& badContainers,
  DeclSet& gcClasses,
  const std::string& outdir,
  bool verbose
) : PluginUtil(context),
    m_rewriter(rewriter),
    m_hasScanMethodSet(hasScanMethod),
    m_badContainers(badContainers),
    m_gcClasses(gcClasses),
    m_outdir(outdir.empty() ? "." : outdir),
    m_verbose(verbose),
    m_resourceDecl(nullptr),
    m_objectDecl(nullptr)
{ }

ScanGenerator::~ScanGenerator() { }

}
