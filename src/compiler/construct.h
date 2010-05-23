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

#ifndef __CONSTRUCT_H__
#define __CONSTRUCT_H__

#include <util/json.h>
#include <compiler/code_generator.h>
#include <compiler/analysis/code_error.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(IParseHandler);
DECLARE_BOOST_TYPES(Location);
DECLARE_BOOST_TYPES(AnalysisResult);

class IParseHandler {
public:
  virtual ~IParseHandler() {}

  /**
   * To avoid iteration of parse tree, we move any work that can be done
   * in parse phase into this function, so to speed up static analysis.
   */
  virtual void onParse(AnalysisResultPtr ar) = 0;
};

/**
 * Base class of Expression and Statement.
 */
class Construct : public boost::enable_shared_from_this<Construct>,
                  public JSON::ISerializable {
public:
  Construct(LocationPtr loc);
  virtual ~Construct() {}

  enum Effect {
    NoEffect = 0,
    IOEffect = 1,                // could have an observable effect (not
                                 // changing variable values)
    AssignEffect = 2,            // writes an object in a way understood by the
                                 // alias manager
    GlobalEffect = 4,            // could affect global variables
    LocalEffect = 8,             // could affect variables from the local scope
    ParamEffect = 0x10,          // a function could affect its parameters
    DeepParamEffect = 0x20,      // a function could affect the array elements
                                 // or object members referenced by its
                                 // parameters
    DynamicParamEffect = 0x40,   // a function creates dynamic exps based
                                 // on its parameters, which it could affect
    CanThrow = 0x80,             // can throw PHP exception
    UnknownEffect = 0x1ff        // anything might happen
  };

  LocationPtr getLocation() { return m_loc;}
  void setLocation(LocationPtr loc) { m_loc = loc;}
  void setFileLevel() { m_topLevel = m_fileLevel = true;}
  void setTopLevel() { m_topLevel = true;}
  bool isFileLevel() const { return m_fileLevel;}
  bool isTopLevel() const { return m_topLevel;}
  virtual int getLocalEffects() const { return UnknownEffect;}
  int getChildrenEffects() const;
  int getContainedEffects() const;
  bool hasEffect() const { return getContainedEffects() != NoEffect;}

  template<typename T>
  static boost::shared_ptr<T> Clone(boost::shared_ptr<T> &constr) {
    if (constr) {
      return boost::dynamic_pointer_cast<T>(constr->clone());
    }
    return boost::shared_ptr<T>();
  }

  /**
   * Called when we analyze a program, which file it includes, which function
   * and class it uses, etc.
   */
  virtual void analyzeProgram(AnalysisResultPtr ar) = 0;

  /**
   * return the nth child construct
   */
  virtual ConstructPtr getNthKid(int n) const { return ConstructPtr(); }

  /**
   * set the nth child construct
   */
  virtual void setNthKid(int n, ConstructPtr cp)  {}

  /**
   * get the kid count
   */
  virtual int getKidCount() const = 0;

  virtual void dump(int spc, AnalysisResultPtr ar);

  /**
   * Called when generating code.
   */
  virtual void outputPHP(CodeGenerator &cg, AnalysisResultPtr ar) = 0;
  virtual void outputCPP(CodeGenerator &cg, AnalysisResultPtr ar) = 0;

  /**
   * Implements JSON::ISerializable.
   */
  virtual void serialize(JSON::OutputStream &out) const;

  /**
   * Get canonicalized PHP source code for this construct.
   */
  std::string getText(bool useCache = false, bool translate = false);

  void addHphpNote(const std::string &s);
  bool hasHphpNote(const std::string &s) const {
    return m_extra && m_extra->hphpNotes.find(s) != m_extra->hphpNotes.end();
  }
  const std::string &getEmbedded() const {
    return getExtra()->embedded;
  }
  void addSuppressError(CodeError::ErrorType e) {
    getExtra()->suppressedErrors.push_back(e);
  }
  bool isErrorSuppressed(CodeError::ErrorType e) const;

  static void recomputeEffects() { s_effectsTag++; }
private:
  struct ExtraData {
    std::set<std::string> hphpNotes;
    std::string embedded;
    std::vector<CodeError::ErrorType> suppressedErrors;
  };
  mutable ExtraData *m_extra;
  std::string m_text;
  ExtraData *getExtra() const {
    if (m_extra == NULL) {
      m_extra = new ExtraData();
    }
    return m_extra;
  }

protected:
  LocationPtr m_loc;
  bool m_fileLevel; // whether or not this is at top level of a file
  bool m_topLevel;  // whether or not this is at top level of a scope
  mutable int m_containedEffects;
  mutable int m_effectsTag;

  static int s_effectsTag;

  /**
   * Called by analyzeProgram() to add a reference to a user class or
   * function.
   */
  void addUserFunction(AnalysisResultPtr ar, const std::string &name,
                       bool strong = true);
  void addUserClass(AnalysisResultPtr ar, const std::string &name,
                    bool strong = true);

  /**
   * Write where this construct was in PHP files.
   */
  void printSource(CodeGenerator &cg);
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CONSTRUCT_H__
