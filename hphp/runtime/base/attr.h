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

#ifndef incl_HPHP_ATTR_H_
#define incl_HPHP_ATTR_H_

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Special properties on PHP classes, functions, and properties.
 *
 * Attr unions are stored as integers in .hhbc repositories, so incompatible
 * changes here require a schema version bump.
 *
 * TODO(#4513748): We're almost out of space in Attr---in fact, we already have
 * too many Attrs to fit in Class, which packs them into only 28 bits.  There's
 * no reason to share Attrs among unrelated objects, so we should really have
 * three different Attr types.
 */
enum Attr {
  AttrNone                 = 0,         // class | property | method  //
                                        //       |          |         //
  // Does this function return by reference?     |          |         //
  AttrReference            = (1 <<  0), //       |          |    X    //
                                        //       |          |         //
  // Method visibility.  The relative ordering of these is important. //
  // N.B. the values are overlayed with some of the no-override bits for magic
  // class methods (next), since they don't apply to classes.
  AttrPublic               = (1 <<  1), //       |    X     |    X    //
  AttrProtected            = (1 <<  2), //       |    X     |    X    //
  AttrPrivate              = (1 <<  3), //       |    X     |    X    //
                                        //       |          |         //
  // No-override bits for magic class methods.  If set, the class does not
  // define that magic function, and neither does any derived class.  Note that
  // the bit for __unset is further down due to Attr-sharing across types.
  AttrNoOverrideMagicGet   = (1 <<  1), //   X   |          |         //
  AttrNoOverrideMagicSet   = (1 <<  2), //   X   |          |         //
  AttrNoOverrideMagicIsset = (1 <<  3), //   X   |          |         //
  // N.B.: AttrEnum and AttrStatic overlap! But they can't be set on the
  // same things.
  // Is this class an enum?
  AttrEnum                 = (1 <<  4), //    X  |          |         //
  // Was this declared static, abstract, or final?          |         //
  AttrStatic               = (1 <<  4), //       |    X     |    X    //
  AttrAbstract             = (1 <<  5), //    X  |          |    X    //
  AttrFinal                = (1 <<  6), //    X  |          |    X    //
                                        //       |          |         //
  // Is this class an interface?        //       |          |         //
  AttrInterface            = (1 <<  7), //    X  |          |         //
                                        //       |          |         //
  // Indicates that a function does not make any explicit calls to other PHP
  // functions.  It may still call other user-level functions via re-entry
  // (e.g., for destructors and autoload), and it may make calls to builtins
  // using FCallBuiltin.                //       |          |         //
  AttrPhpLeafFn            = (1 <<  7), //       |          |    X    //
                                        //       |          |         //
  // Is this class a trait?  On methods, this indicates that the method is NOT
  // a constructor, even though it may look like one.  FIXME: This is insane.
  AttrTrait                = (1 <<  8), //    X  |          |    X    //
                                        //       |          |         //
  // Indicates that this function should be ignored in backtraces.    //
  AttrNoInjection          = (1 <<  9), //       |          |    X    //
  // Indicates a class has no derived classes that have a magic __unset method.
  AttrNoOverrideMagicUnset = (1 <<  9), //   X   |          |         //
                                        //       |          |         //
  // Indicates that the function or class is uniquely named among functions or
  // classes across the codebase.  Note that function and class names are in
  // separate namespaces, so it is possible to have a Func and Class which
  // share a name but both of which are unique.  |          |         //
  AttrUnique               = (1 << 10), //    X  |          |    X    //
                                        //       |          |         //
  // Only valid in RepoAuthoritative mode.  Indicates that a function can be
  // used with fb_rename_function---even if JitEnableRenameFunction is
  // false---and can be used with fb_intercept.  (Note: we could split this
  // into two bits, since you can technically pessimize less for fb_intercept
  // than you need to for fb_rename_function, but we haven't done so at this
  // point.)                            //       |          |         //
  AttrInterceptable        = (1 << 11), //       |          |    X    //
                                        //       |          |         //
  // FIXME: I have no documentation.    //       |          |         //
  AttrNoExpandTrait        = (1 << 12), //    X  |          |         //
                                        //       |          |         //
  // Set on functions where the $this class may not be a subclass of the
  // context (scope) class.
  AttrHasForeignThis       = (1 << 12), //       |          |    X    //
                                        //       |          |         //
  // Only valid in WholeProgram mode.  Indicates on a class that the class is
  // not extended, or on a method that no extending class defines the method.
  AttrNoOverride           = (1 << 13), //    X  |          |    X    //
                                        //       |          |         //
  // Indicates that this method should always be cloned when inherited.
  AttrClone                = (1 << 14), //       |          |    X    //
                                        //       |          |         //
  // Indicates that a function is a builtin that takes variadic arguments,
  // where the arguments are either by ref or optionally by ref.  It is
  // equivalent to ClassInfo's (RefVariableArguments).
  AttrVariadicByRef        = (1 << 15), //       |          |    X    //
                                        //       |          |         //
  // Indicates that a function may need to use a VarEnv or varargs (i.e.,
  // extraArgs) at runtime.  If the debugger is enabled, all functions
  // must be treated as having this flag.
  AttrMayUseVV             = (1 << 16), //       |          |    X    //
                                        //       |          |         //
  // Indicates that the function or class can be loaded once and then persisted
  // across all requests.               //       |          |         //
  AttrPersistent           = (1 << 17), //    X  |          |    X    //
                                        //       |          |         //
  // Indicates that this property cannot be initialized on an ObjectData by
  // simply memcpy-ing from the initializer vector.         |         //
  AttrDeepInit             = (1 << 18), //       |    X     |         //
                                        //       |          |         //
  // This HNI method takes an additional "func_num_args()" value at the
  // beginning of its signature (after Class*/ObjectData* for methods)
  AttrNumArgs              = (1 << 18), //       |          |    X    //
                                        //       |          |         //
  // Set on functions to mark them as hot during PGO profiling.       //
  AttrHot                  = (1 << 19), //       |          |    X    //
                                        //       |          |         //
  // Set on all builtin functions, whether PHP or C++. For properties, it
  // is set on internal properties (e.g. <<__memoize>> caching)
  AttrBuiltin              = (1 << 20), //    X  |    X     |    X    //
                                        //       |          |         //
  // Set on builtin functions that can be replaced by user implementations.
  AttrAllowOverride        = (1 << 21), //       |          |    X    //
                                        //       |          |         //
  // Indicates that the frame should be ignored when searching for context
  // (e.g., array_map evalutates its callback in the context of the caller).
  AttrSkipFrame            = (1 << 22), //       |          |    X    //
                                        //       |          |         //
  // Is this an HNI builtin?            //       |          |         //
  AttrNative               = (1 << 23), //       |          |    X    //
                                        //       |          |         //
  // Indicates that this function can be constant-folded if it is called with
  // all constant arguments.            //       |          |         //
  AttrIsFoldable           = (1 << 26), //       |          |    X    //
                                        //       |          |         //
  // Indicates that this function cannot be called with FCallBuiltin because it
  // requires an ActRec argument.       //       |          |         //
  AttrNoFCallBuiltin       = (1 << 27), //       |          |    X    //
                                        //       |          |         //
  // Does this function have a `...' parameter?  |          |         //
  AttrVariadicParam        = (1 << 28), //       |          |    X    //
                                        //       |          |         //
  // Indicates that a function will attempt to coerce parameters to the correct
  // type.  This isn't the same as casting---for example, a string can be cast
  // to an array, but cannot be coerced to an array.  If it fails, the function
  // returns either false or null, depending on the mode.  This behavior is
  // common in PHP5 builtins.           //       |          |         //
  AttrParamCoerceModeFalse = (1 << 29), //       |          |    X    //
  AttrParamCoerceModeNull  = (1 << 30), //       |          |    X    //
};

inline Attr operator|(Attr a, Attr b) { return Attr((int)a | (int)b); }

inline Attr& operator|=(Attr& a, const Attr& b) {
  return (a = Attr((int)a | (int)b));
}

inline const char* attrToVisibilityStr(Attr attr) {
  return (attr & AttrPrivate)   ? "private"   :
         (attr & AttrProtected) ? "protected" : "public";
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ATTR_H_
