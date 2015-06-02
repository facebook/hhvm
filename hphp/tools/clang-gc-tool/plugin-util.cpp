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

#include "hphp/tools/clang-gc-tool/plugin-util.h"
#include <iostream>
#include <string>
#include "llvm/Support/raw_ostream.h"

namespace HPHP {

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
using clang::SourceRange;
using clang::isa;
using clang::cast;
using clang::dyn_cast;
using clang::dyn_cast_or_null;
using clang::PrintingPolicy;
using clang::RawComment;
using clang::TTK_Class;
using clang::TTK_Union;
using clang::TTK_Struct;

PluginUtil::PluginUtil(ASTContext& context)
: m_context(context) { }

PluginUtil::~PluginUtil() { }

bool PluginUtil::isCppFile(const char* file) const {
  const char* dot = strrchr(file, '.');
  if (!dot) {
    warning("malformed file name (no suffix): %s", file);
    return false;
  }
  return dot[1] == 'c';
}

bool PluginUtil::isHeaderFile(const char* file) const {
  const char* dot = strrchr(file, '.');
  if (!dot) {
    warning("malformed file name (no suffix): %s", file);
    return false;
  }
  return dot[1] == 'h';
}

bool PluginUtil::isHiddenDecl(const CXXRecordDecl* decl) const {
  auto def = decl->getDefinition();
  return isCppFile(getFilename(def ? def : decl));
}

const char* PluginUtil::getFilename(const SourceRange& rng) const {
  const auto& mgr = m_context.getSourceManager();
  auto loc = mgr.getPresumedLoc(rng.getBegin());
  return loc.getFilename();
}

const char* PluginUtil::getFilename(const Decl* decl) const {
  return getFilename(decl->getSourceRange());
}

const char* PluginUtil::getDefinitionFilename(const Decl* decl) const {
  return getFilename(cast<CXXRecordDecl>(decl)->getDefinition());
}

bool PluginUtil::inSystemHeader(const Decl* decl) const {
  auto fullLoc = m_context.getFullLoc(decl->getSourceRange().getBegin());
  return fullLoc.isInSystemHeader();
}

std::string PluginUtil::locToString(const SourceRange& rng) const {
  const auto& mgr = m_context.getSourceManager();
  auto loc = mgr.getPresumedLoc(rng.getBegin());
  const char* file = loc.getFilename();
  auto line = loc.getLine();
  auto col = loc.getColumn();
  return boost::str(boost::format("%s:%d:%d:") % file % line % col);
}

void
PluginUtil::error(const std::string& locStr, const std::string& msg) const {
  std::cerr << locStr << "error: " << msg << "\n";
  exit(1);
}

void
PluginUtil::warning(const std::string& locStr, const std::string& msg) const {
  Lockfile lf("messages", false);
  std::cout << locStr << "warning: " << msg << "\n" << std::flush;
}

void PluginUtil::warning(const DeclMap& whys,
                         const NamedDecl* decl,
                         const std::string& msg) const {
  Lockfile lf("messages", false);
  std::cout << locToString(decl->getSourceRange()) << "warning: " << msg;
  const char* first = " (";
  do {
    std::cout << first;
    if (auto field = dyn_cast<FieldDecl>(decl)) {
      std::cout << "contains field '"
                << getName(field) << ":"
                << getName(field->getType()) << "'";
      const auto& type = stripType(*field->getType());
      auto recordDecl = type.getAsCXXRecordDecl();
      decl = recordDecl ? recordDecl->getCanonicalDecl() : recordDecl;
    } else {
      std::cout << "is subclass of '" << getName(decl) << "'";
    }
    decl = exists(whys, decl) ? whys.find(decl)->second : nullptr;
    first = ", ";
  } while (decl);
  if (*first != '(') {
    std::cout << ")";
  }
  std::cout << "\n" << std::flush;
}

const Type& PluginUtil::stripType(const Type& ty) {
  if (isPointerOrReferenceType(ty)) {
    return stripType(getPointeeType(ty));
  } else if (isArrayType(ty)) {
    return stripType(getElementType(ty));
  } else {
    return ty;
  }
}

const CXXRecordDecl*
PluginUtil::getCanonicalDef(const CXXRecordDecl* decl) const {
  if(decl) {
    auto canon = decl->getCanonicalDecl();
    return canon->getDefinition();
  }
  return nullptr;
}

bool PluginUtil::isPointerType(const Type& ty) {
  return isa<PointerType>(&ty);
}

bool PluginUtil::isReferenceType(const Type& ty) {
  return isa<ReferenceType>(&ty);
}

bool PluginUtil::isPointerOrReferenceType(const Type& ty) {
  return isPointerType(ty) || isReferenceType(ty);
}

bool PluginUtil::isArrayType(const Type& ty) {
  return isa<ArrayType>(&ty);
}

bool PluginUtil::isPOD(const clang::Type& ty) {
  auto decl = ty.getAsCXXRecordDecl();
  return decl && decl->isPOD();
}

bool PluginUtil::isSubclassOf(const NamedDecl* maybeBase,
                              const CXXRecordDecl* derived) {
  if(auto base = dyn_cast_or_null<CXXRecordDecl>(maybeBase)) {
    return (base == derived ||
            derived->isDerivedFrom(base) ||
            derived->isVirtuallyDerivedFrom(base));
  }
  return false;
}

bool PluginUtil::isSubclassOf(const DeclSet& maybeBases,
                              const CXXRecordDecl* derived) {
  for (auto maybeBase : maybeBases) {
    if (isSubclassOf(maybeBase, derived)) return true;
  }
  return false;
}

const Type& PluginUtil::getElementType(const Type& ty) {
  assert(isArrayType(ty));
  return *cast<ArrayType>(ty).getElementType();
}

const Type& PluginUtil::getPointeeType(const Type& ty) {
  if (isPointerType(ty)) {
    return *ty.getPointeeType();
  } else {
    assert(isReferenceType(ty));
    return *cast<ReferenceType>(&ty)->getPointeeType();
  }
}

bool PluginUtil::isTemplate(const CXXRecordDecl* decl) {
  return (decl->getDescribedClassTemplate() ||
          isa<ClassTemplateSpecializationDecl>(decl));
}

std::string PluginUtil::getTemplateClassName(const CXXRecordDecl* decl,
                                             bool no_namespaces) const {
  assert(isTemplate(decl));
  auto name = getName(decl, no_namespaces);
  return name.substr(0, name.find('<'));
}

const ClassTemplateDecl* PluginUtil::getTemplateDef(const CXXRecordDecl* decl) {
  assert(isTemplate(decl));
  if (decl->getDescribedClassTemplate()) {
    return decl->getCanonicalDecl()->getDescribedClassTemplate();
  } else {
    assert(isa<ClassTemplateSpecializationDecl>(decl));
    auto tempDecl = cast<ClassTemplateSpecializationDecl>(decl);
    return tempDecl->getSpecializedTemplate();
  }
}

void PluginUtil::forEachTemplateParam(
  const CXXRecordDecl* decl,
  const std::function<bool(const NamedDecl*)>& fn
) {
  assert(isTemplate(decl));
  auto templat = getTemplateDef(decl);
  assert(templat);
  for (auto param : (*templat->getTemplateParameters())) {
    if (!fn(param)) break;
  }
}

std::string PluginUtil::getName(QualType ty,
                                bool no_namespaces,
                                bool suppress_tag,
                                bool suppress_qualifiers) const {
  PrintingPolicy pp(m_context.getLangOpts());
  pp.SuppressTagKeyword = suppress_tag;
  pp.SuppressScope = no_namespaces;
  if (suppress_qualifiers) {
    return ty.getUnqualifiedType().getAsString(pp);
  }
  return ty.getAsString(pp);
}

std::string PluginUtil::getName(const Type& ty,
                                bool no_namespaces,
                                bool suppress_tag) const {
  return getName(QualType(&ty, 0), no_namespaces, suppress_tag);
}

std::string PluginUtil::getName(const CXXRecordDecl* decl,
                                bool no_namespaces,
                                bool suppress_tag) const {
  return getName(QualType(decl->getTypeForDecl(), 0),
                 no_namespaces,
                 suppress_tag);
}

std::string PluginUtil::getName(const ClassTemplateDecl* decl,
                                bool no_namespaces,
                                bool suppress_tag) const {
  std::string name;
  llvm::raw_string_ostream ss(name);
  PrintingPolicy p(m_context.getLangOpts());
  p.SuppressTagKeyword = suppress_tag;
  p.SuppressScope = no_namespaces;
  decl->getNameForDiagnostic(ss, p, true);
  return ss.str();
}

std::string PluginUtil::getName(const NamedDecl* decl,
                                bool no_namespaces,
                                bool suppress_tag) const {
  if (auto cdecl = dyn_cast<CXXRecordDecl>(decl)) {
    return getName(cdecl, no_namespaces, suppress_tag);
  }
  if (auto tdecl = dyn_cast<ClassTemplateDecl>(decl)) {
    return getName(tdecl, no_namespaces, suppress_tag);
  }
  return decl->getDeclName().getAsString();
}

std::string PluginUtil::tagName(const CXXRecordDecl* decl) const {
  switch (decl->getTagKind()) {
    case TTK_Struct: return "struct";
    case TTK_Class: return "class";
    case TTK_Union: return "union";
    default: assert(0); return "";
  }
  assert(0);
  return "";
}

bool PluginUtil::isNestedDecl(const CXXRecordDecl* decl) const {
  auto ctx = decl->getDeclContext();
  return ctx && ctx->isRecord();
}

std::vector<const NamespaceDecl*>
PluginUtil::getParentNamespaces(const CXXRecordDecl* def) {
  std::vector<const NamespaceDecl*> nsvec;
  auto p = def->getDeclContext();
  while (p) {
    if (isa<NamespaceDecl>(p)) {
      auto ns = cast<NamespaceDecl>(p);
      if (!ns->isAnonymousNamespace()) {
        nsvec.push_back(ns);
      }
    }
    p = p->getParent();
  }
  std::reverse(nsvec.begin(), nsvec.end());
  return nsvec;
}

std::vector<const CXXRecordDecl*>
PluginUtil::getOuterClasses(const CXXRecordDecl* def) {
  std::vector<const CXXRecordDecl*> rvec;
  auto p = def->getDeclContext();
  while (p) {
    if(auto rec = dyn_cast<CXXRecordDecl>(p)) {
      if (!rec->isAnonymousStructOrUnion()) {
        rvec.push_back(rec);
      }
    }
    p = p->getParent();
  }
  std::reverse(rvec.begin(), rvec.end());
  return rvec;
}

// Find comment (if any) immediately preceding decl.
const RawComment* PluginUtil::findComment(const Decl* decl) const {
  const auto& mgr = m_context.getSourceManager();
  auto declBeg = decl->getSourceRange().getBegin();
  for (auto comment : m_comments) {
    auto comEnd = comment->getSourceRange().getEnd();
    // crude adjacency check....
    if (comEnd < declBeg &&
        mgr.getSpellingLineNumber(comEnd) + 1 ==
        mgr.getSpellingLineNumber(declBeg)) {
      return comment;
    }
  }
  return nullptr;
}

void PluginUtil::collectComments() {
  // Collect all comments.
  for (const auto* comment : m_context.getRawCommentList().getComments()) {
    m_comments.insert(comment);
  }
}

}
