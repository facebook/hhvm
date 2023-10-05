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

#pragma once

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Special properties on PHP classes, functions, and properties.
 *
 * Attr unions are stored as integers in .hhbc repositories, so incompatible
 * changes here require a schema version bump.
 *
 * NOTE: Make sure to keep this in sync with HHAS_ATTRS in as-base-hhas.h so it
 * prints properly.
 *
 * TODO(#4513748): We're almost out of space in Attr---in fact, we already have
 * too many Attrs to fit in Class, which packs them into only 28 bits.  There's
 * no reason to share Attrs among unrelated objects, so we should really have
 * three different Attr types.
 */
enum Attr {
  AttrNone                 = 0,          // class | property | method  //
                                         //       |          |         //
  // Class forbids dynamic properties?   //       |          |         //
  AttrForbidDynamicProps   = (1u <<  0), //   X   |          |         //
                                         //       |          |         //
  // Indicates that this property cannot be initialized on an ObjectData by
  // simply memcpy-ing from the initializer vector.          |         //
  AttrDeepInit             = (1u <<  0), //       |    X     |         //
                                         //       |          |         //
  // Method visibility.  The relative ordering of these is important.  //
  // N.B. the values are overlayed with some of the no-override bits for magic
  // class methods (next), since they don't apply to classes.
  AttrPublic               = (1u <<  1), //       |    X     |    X    //
  AttrProtected            = (1u <<  2), //       |    X     |    X    //
  AttrPrivate              = (1u <<  3), //       |    X     |    X    //
                                         //       |          |         //
  // N.B.: AttrEnum and AttrStatic overlap! But they can't be set on the
  // same things.
  // Is this class an enum?
  AttrEnum                 = (1u <<  4), //    X  |          |         //
  // Was this property's initial value supplied by the emitter (rather than a
  // user). System provided initial values can be modified to match the
  // property's type-hint.
  AttrSystemInitialValue   = (1u <<  5), //       |    X     |         //
  // Normally properties might contain KindOfNull values, even if their
  // type-hint doesn't allow this (because of initial values). This indicates
  // the property won't contain KindOfNull if its type-hint doesn't allow it.
  AttrNoImplicitNullable   = (1u <<  6), //       |    X     |         //
  // Was this declared static, abstract, or final?
  AttrStatic               = (1u <<  4), //       |    X     |    X    //
  AttrAbstract             = (1u <<  5), //    X  |          |    X    //
  AttrFinal                = (1u <<  6), //    X  |          |    X    //
                                         //       |          |         //
  // Is this class an interface?         //       |          |         //
  AttrInterface            = (1u <<  7), //    X  |          |         //
                                         //       |          |         //
  // Indicates that a static property has the <<__LSB>> attribute.
  // Such a property is implicitly redeclared in all derived classes.
  AttrLSB                  = (1u <<  7), //       |    X     |         //
  // Does this function support the async eager return optimization? If so,
  // instead of returning a finished Awaitable, this function may return the
  // unpacked result of the Awaitable, assuming the AsyncEagerRet ActRec flag
  // was set by the caller.
  AttrSupportsAsyncEagerReturn
                           = (1u <<  7), //       |          |    X    //
  // Is this class a trait?  On methods, or properties, this indicates that
  // the method was imported from a trait.
  AttrTrait                = (1u <<  8), //    X  |    X     |    X    //
                                         //       |          |         //
  // Indicates that this function should be ignored in backtraces.     //
  AttrNoInjection          = (1u <<  9), //       |          |    X    //
                                         //       |          |         //
  // Indicates this property's initial value satisfies its type-constraint and
  // no runtime check needs to be done.
  AttrInitialSatisfiesTC   = (1u <<  9), //       |    X     |         //
  // Indicates this class or any of its subclasses is not mocked.
  AttrNoMock               = (1u <<  9), //    X  |          |         //
  // Indicates that this property is definitely not redeclaring a property in a
  // parent, or if it is, the type-hints of the two properties are equivalent
  // (and therefore requires no runtime check).
  AttrNoBadRedeclare       = (1u << 10), //       |    X     |         //
  // Indicates that a function can be used with fb_intercept2.
  AttrInterceptable        = (1u << 11), //       |          |    X    //
                                         //       |          |         //
  // This class is sealed                //       |          |         //
  AttrSealed               = (1u << 11), //    X  |          |         //
  // Property starts as uninit, will throw if accessed before being explicitly
  // set.
  AttrLateInit             = (1u << 11), //       |    X     |         //
  // Traits have been flattened on this class.
  AttrNoExpandTrait        = (1u << 12), //    X  |          |         //
                                         //       |          |         //
  // Indicates that the class is not extended, or on a method that no
  // extending class defines the method. Implies
  // AttrNoOverrideOverride for classes.
  AttrNoOverride           = (1u << 13), //    X  |          |    X    //
                                         //       |          |         //
  // Set on classes to indicate it is not extended by a "regular"
  // class (but might be extended by non-regular classes). A regular
  // class is a class which isn't a trait, enum, or abstract
  // class. Such classes cannot be instantiated (but can be
  // manipulated in static contexts).  This is a weaker condition than
  // AttrNoOverride but is useful if you have a known instance of the
  // class (and therefore must be regular).
  AttrNoOverrideRegular    = (1u << 14), //    X  |          |         //
                                         //       |          |         //
  // Indicates that this property was declared as readonly             //
  AttrIsReadonly           = (1u << 14), //       |    X     |         //
                                         //       |          |         //
  // Indicates that the function does not modify its instance
  AttrReadonlyThis         = (1u << 14), //       |          |    X    //
                                         //       |          |         //
  // Indicates that the function returns readonly value
  AttrReadonlyReturn       = (1u << 15), //       |          |    X    //
                                         //       |          |         //
  // Indicates that this symbol is internal to the module it is declared in
  AttrInternal             = (1u << 16), //    X  |    X     |    X    //
                                         //       |          |         //
  // Same as AttrInternal except when producing an error, instead of
  // throwing, it always raises a warning
  AttrInternalSoft         = (1u << 17), //    X  |    X     |    X    //
                                         //       |          |         //
  // Indicates that the function, class or static property can be loaded
  // once and then persisted across all requests. |          |         //
  AttrPersistent           = (1u << 18), //    X  |    X     |    X    //
                                         //       |          |         //
  // Set on functions to mark them as being able to be dynamically called
  AttrDynamicallyCallable  = (1u << 19), //       |          |    X    //
                                         //       |          |         //
  // Set on classes to mark them as being able to be dynamically constructed
  AttrDynamicallyConstructible
                           = (1u << 19), //    X  |          |         //
                                         //       |          |         //
  // Set on all builtin functions, whether PHP or C++.
  AttrBuiltin              = (1u << 20), //    X  |          |    X    //
                                         //       |          |         //
  // Set on functions that have 86productAttributionData variable      //
  AttrHasAttributionData   = (1u << 21), //       |          |    X    //
                                         //       |          |         //
  // Set on properties to indicate they can't be changed after construction
  // and on classes to indicate that all that class' properties are const.
  AttrIsConst              = (1u << 21), //    X  |    X     |         //
                                         //       |          |         //
  // Set on classes where a user is allowed get a pointer to the class from a
  // string.
  AttrDynamicallyReferenced= (1u << 22), //    X  |          |         //
  // Set on base classes that do not have any reified classes that extend it.
  AttrNoReifiedInit        = (1u << 23), //    X  |          |         //
                                         //       |          |         //
  AttrIsMethCaller         = (1u << 24), //       |          |    X    //
                                         //       |          |         //
  // Set on closure classes                                            //
  AttrIsClosureClass       = (1u << 24), //    X  |          |         //
  // Set on closure classes that use a property to store required coeffects
  AttrHasClosureCoeffectsProp                                          //
                           = (1u << 25), //    X  |          |         //
  // Set on functions with coeffect rules
  AttrHasCoeffectRules     = (1u << 25), //       |          |    X    //
                                         //       |          |         //
  // Indicates that this function can be constant-folded if it is called with
  // all constant arguments.             //       |          |         //
  AttrIsFoldable           = (1u << 26), //       |          |    X    //
                                         //       |          |         //
  // Indicates that this function cannot be called with FCallBuiltin because it
  // requires an ActRec argument.        //       |          |         //
  AttrNoFCallBuiltin       = (1u << 27), //       |          |    X    //
                                         //       |          |         //
  // Does this function have a `...' parameter?   |          |         //
  AttrVariadicParam        = (1u << 28), //       |          |    X    //
                                         //       |          |         //
  // Indicates that the frame should be ignored when searching for a context to
  // store in the provenance tag.  (For HNI builtins, indicates that we should
  // skip tagging the return value with the builtin's callsite.)
  AttrProvenanceSkipFrame  = (1u << 29), //       |          |    X    //
                                         //       |          |         //
  // Is this an "enum class" (in the sense of the "enum dependent types" feature)?
  AttrEnumClass            = (1u << 30), //    X  |          |         //
                                         //       |          |         //
  // Is this a native function whose calls should not be included in recordings?
  AttrNoRecording          = (1u << 30), //       |          |    X    //
                                         //       |          |         //
  // XXX: The enum is used as a bitmask and without a value in the highest bit
  //      we get assertions in dev builds.
  AttrUnusedMaxAttr        = (1u << 31),
};

constexpr Attr operator|(Attr a, Attr b) { return Attr((int)a | (int)b); }

inline Attr& operator|=(Attr& a, const Attr& b) {
  return (a = Attr((int)a | (int)b));
}

inline void attrSetter(Attr& attrs, bool set, Attr what) {
  if (set) {
    attrs |= what;
  } else {
    attrs = Attr(attrs & ~what);
  }
}

constexpr Attr VisibilityAttrs = AttrPublic|AttrProtected|AttrPrivate;

inline const char* attrToVisibilityStr(Attr attr) {
  return (attr & AttrPrivate)   ? "private"   :
         (attr & AttrProtected) ? "protected" : "public";
}

///////////////////////////////////////////////////////////////////////////////
}
