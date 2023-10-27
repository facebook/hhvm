(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(** Module consisting of the special names known to the typechecker *)

module Classes = struct
  let cParent = "parent"

  let cStatic = "static"

  let cSelf = "self"

  let cUnknown = "\\*Unknown*"

  (* Used for dynamic classnames, e.g. new $foo(); *)

  let cAwaitable = "\\HH\\Awaitable"

  let cGenerator = "\\Generator"

  let cAsyncGenerator = "\\HH\\AsyncGenerator"

  let cHHFormatString = "\\HH\\FormatString"

  let is_format_string x = String.equal x cHHFormatString

  let cHH_BuiltinEnum = "\\HH\\BuiltinEnum"

  let cHH_BuiltinEnumClass = "\\HH\\BuiltinEnumClass"

  let cHH_BuiltinAbstractEnumClass = "\\HH\\BuiltinAbstractEnumClass"

  let cThrowable = "\\Throwable"

  let cStdClass = "\\stdClass"

  let cDateTime = "\\DateTime"

  let cDateTimeImmutable = "\\DateTimeImmutable"

  let cAsyncIterator = "\\HH\\AsyncIterator"

  let cAsyncKeyedIterator = "\\HH\\AsyncKeyedIterator"

  let cStringish = "\\Stringish"

  let cStringishObject = "\\StringishObject"

  let cXHPChild = "\\XHPChild"

  let cIMemoizeParam = "\\HH\\IMemoizeParam"

  let cUNSAFESingletonMemoizeParam = "\\HH\\UNSAFESingletonMemoizeParam"

  let cClassname = "\\HH\\classname"

  let cTypename = "\\HH\\typename"

  let cIDisposable = "\\IDisposable"

  let cIAsyncDisposable = "\\IAsyncDisposable"

  let cMemberOf = "\\HH\\MemberOf"

  let cEnumClassLabel = "\\HH\\EnumClass\\Label"

  (* Classes that can be spliced into ExpressionTrees *)
  let cSpliceable = "\\Spliceable"

  let cSupportDyn = "\\HH\\supportdyn"
end

module Collections = struct
  (* concrete classes *)
  let cVector = "\\HH\\Vector"

  let cMutableVector = "\\MutableVector"

  let cImmVector = "\\HH\\ImmVector"

  let cSet = "\\HH\\Set"

  let cConstSet = "\\ConstSet"

  let cMutableSet = "\\MutableSet"

  let cImmSet = "\\HH\\ImmSet"

  let cMap = "\\HH\\Map"

  let cMutableMap = "\\MutableMap"

  let cImmMap = "\\HH\\ImmMap"

  let cPair = "\\HH\\Pair"

  (* interfaces *)
  let cContainer = "\\HH\\Container"

  let cKeyedContainer = "\\HH\\KeyedContainer"

  let cTraversable = "\\HH\\Traversable"

  let cKeyedTraversable = "\\HH\\KeyedTraversable"

  let cCollection = "\\Collection"

  let cConstVector = "\\ConstVector"

  let cConstMap = "\\ConstMap"

  let cConstCollection = "\\ConstCollection"

  let cAnyArray = "\\HH\\AnyArray"

  let cDict = "\\HH\\dict"

  let cVec = "\\HH\\vec"

  let cKeyset = "\\HH\\keyset"
end

module Members = struct
  let mGetInstanceKey = "getInstanceKey"

  let mClass = "class"

  let __construct = "__construct"

  let __destruct = "__destruct"

  let __call = "__call"

  let __callStatic = "__callStatic"

  let __clone = "__clone"

  let __debugInfo = "__debugInfo"

  let __dispose = "__dispose"

  let __disposeAsync = "__disposeAsync"

  let __get = "__get"

  let __invoke = "__invoke"

  let __isset = "__isset"

  let __set = "__set"

  let __set_state = "__set_state"

  let __sleep = "__sleep"

  let __toString = "__toString"

  let __unset = "__unset"

  let __wakeup = "__wakeup"

  let as_set =
    List.fold_right
      ~f:SSet.add
      ~init:SSet.empty
      [
        __construct;
        __destruct;
        __call;
        __callStatic;
        __clone;
        __debugInfo;
        __dispose;
        __disposeAsync;
        __get;
        __invoke;
        __isset;
        __set;
        __set_state;
        __sleep;
        __toString;
        __unset;
        __wakeup;
      ]

  let as_lowercase_set = SSet.map String.lowercase as_set

  (* Any data- or aria- attribute is always valid, even if it is not declared
   * for a given XHP element *)
  let is_special_xhp_attribute s =
    String.is_prefix s ~prefix:":data-" || String.is_prefix s ~prefix:":aria-"
end

module AttributeKinds = struct
  let cls = "\\HH\\ClassAttribute"

  let clscst = "\\HH\\ClassConstantAttribute"

  let enum = "\\HH\\EnumAttribute"

  let typealias = "\\HH\\TypeAliasAttribute"

  let fn = "\\HH\\FunctionAttribute"

  let mthd = "\\HH\\MethodAttribute"

  let instProperty = "\\HH\\InstancePropertyAttribute"

  let staticProperty = "\\HH\\StaticPropertyAttribute"

  let parameter = "\\HH\\ParameterAttribute"

  let typeparam = "\\HH\\TypeParameterAttribute"

  let file = "\\HH\\FileAttribute"

  let typeconst = "\\HH\\TypeConstantAttribute"

  let lambda = "\\HH\\LambdaAttribute"

  let enumcls = "\\HH\\EnumClassAttribute"

  let module_ = "\\HH\\ModuleAttribute"

  let plain_english_map =
    List.fold_left
      ~init:SMap.empty
      ~f:(fun acc (k, v) -> SMap.add k v acc)
      [
        (cls, "a class");
        (clscst, "a constant of a class");
        (enum, "an enum");
        (typealias, "a typealias");
        (fn, "a function");
        (mthd, "a method");
        (instProperty, "an instance property");
        (staticProperty, "a static property");
        (parameter, "a parameter");
        (typeparam, "a type parameter");
        (file, "a file");
        (typeconst, "a type constant");
        (lambda, "a lambda expression");
        (enumcls, "an enum class");
        (module_, "a module");
      ]
end

module UserAttributes = struct
  let uaOverride = "__Override"

  let uaConsistentConstruct = "__ConsistentConstruct"

  let uaConst = "__Const"

  let uaDeprecated = "__Deprecated"

  let uaDocs = "__Docs"

  let uaEntryPoint = "__EntryPoint"

  let uaMemoize = "__Memoize"

  let uaMemoizeLSB = "__MemoizeLSB"

  let uaPHPStdLib = "__PHPStdLib"

  let uaAcceptDisposable = "__AcceptDisposable"

  let uaReturnDisposable = "__ReturnDisposable"

  let uaLSB = "__LSB"

  let uaSealed = "__Sealed"

  let uaLateInit = "__LateInit"

  let uaNewable = "__Newable"

  let uaEnforceable = "__Enforceable"

  let uaExplicit = "__Explicit"

  let uaNonDisjoint = "__NonDisjoint"

  let uaSoft = "__Soft"

  let uaWarn = "__Warn"

  let uaMockClass = "__MockClass"

  let uaProvenanceSkipFrame = "__ProvenanceSkipFrame"

  let uaDynamicallyCallable = "__DynamicallyCallable"

  let uaDynamicallyConstructible = "__DynamicallyConstructible"

  let uaDynamicallyReferenced = "__DynamicallyReferenced"

  let uaReifiable = "__Reifiable"

  let uaNeverInline = "__NEVER_INLINE"

  let uaDisableTypecheckerInternal = "__DisableTypecheckerInternal"

  let uaHasTopLevelCode = "__HasTopLevelCode"

  let uaIsFoldable = "__IsFoldable"

  let uaNative = "__Native"

  let uaNativeData = "__NativeData"

  let uaEagerVMSync = "__EagerVMSync"

  let uaAutocompleteSortText = "__AutocompleteSortText"

  let uaOutOnly = "__OutOnly"

  let uaAlwaysInline = "__ALWAYS_INLINE"

  let uaEnableUnstableFeatures = "__EnableUnstableFeatures"

  let uaEnumClass = "__EnumClass"

  let uaPolicied = "__Policied"

  let uaInferFlows = "__InferFlows"

  let uaExternal = "__External"

  let uaCanCall = "__CanCall"

  let uaSupportDynamicType = "__SupportDynamicType"

  let uaNoAutoDynamic = "__NoAutoDynamic"

  let uaNoAutoLikes = "__NoAutoLikes"

  let uaNoAutoBound = "__NoAutoBound"

  let uaRequireDynamic = "__RequireDynamic"

  let uaEnableMethodTraitDiamond = "__EnableMethodTraitDiamond"

  let uaIgnoreReadonlyLocalErrors = "__IgnoreReadonlyLocalErrors"

  let uaIgnoreCoeffectLocalErrors = "__IgnoreCoeffectLocalErrors"

  let uaReified = "__Reified"

  let uaHasReifiedParent = "__HasReifiedParent"

  let uaSoftInternal = "__SoftInternal"

  let uaNoFlatten = "__NoFlatten"

  let uaCrossPackage = "__CrossPackage"

  (* <<__SafeForGlobalAccessCheck>> marks global variables as safe from mutations.
     This attribute merely ensures that the global_access_check does NOT raise
     errors/warnings from writing to the annotated global variable, and it
     has NO runtime/semantic implication. *)
  let uaSafeGlobalVariable = "__SafeForGlobalAccessCheck"

  let uaModuleLevelTrait = "__ModuleLevelTrait"

  let uaStrictSwitch = "__StrictSwitch"

  type attr_info = {
    contexts: string list;
    doc: string;
    autocomplete: bool;
  }

  let as_map : attr_info SMap.t =
    AttributeKinds.(
      SMap.of_list
        [
          ( uaOverride,
            {
              contexts = [mthd];
              autocomplete = true;
              doc = "Ensures there's a parent method being overridden.";
            } );
          ( uaConsistentConstruct,
            {
              contexts = [cls];
              autocomplete = true;
              doc =
                "Requires all child classes to have the same constructor signature. "
                ^ " This allows `new static(...)` and `new $the_class_name(...)`.";
            } );
          ( uaConst,
            {
              contexts = [cls; instProperty; parameter; staticProperty];
              autocomplete = false;
              doc =
                "Marks a class or property as immutable."
                ^ " When applied to a class, all the properties are considered `__Const`."
                ^ " `__Const` properties can only be set in the constructor.";
            } );
          ( uaDeprecated,
            {
              contexts = [fn; mthd];
              autocomplete = true;
              doc =
                "Mark a function/method as deprecated. "
                ^ " The type checker will show an error at call sites, and a runtime notice is raised if this function/method is called."
                ^ "\n\nThe optional second argument specifies a sampling rate for raising notices at runtime."
                ^ " If the sampling rate is 100, a notice is only raised every 1/100 calls. If omitted, the default sampling rate is 1 (i.e. all calls raise notices)."
                ^ " To disable runtime notices, use a sampling rate of 0.";
            } );
          ( uaDocs,
            {
              contexts = [cls; enum; enumcls; typealias];
              autocomplete = true;
              doc = "Shows the linked URL when hovering over this type.";
            } );
          ( uaAutocompleteSortText,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc =
                "Declares which string will be used for ordering this in autocomplete suggestions.";
            } );
          ( uaEntryPoint,
            {
              contexts = [fn];
              autocomplete = true;
              doc =
                "Execution of the program will start here."
                ^ Printf.sprintf
                    " This only applies in the first file executed, `%s` in required or autoloaded files has no effect."
                    uaEntryPoint;
            } );
          ( uaMemoize,
            {
              contexts = [fn; mthd];
              autocomplete = true;
              doc =
                "Cache the return values from this function/method."
                ^ " Calls with the same arguments will return the cached value."
                ^ "\n\nCaching is per-request and shared between subclasses (see also `__MemoizeLSB`).";
            } );
          ( uaMemoizeLSB,
            {
              contexts = [mthd];
              autocomplete = true;
              doc =
                "Cache the return values from this method."
                ^ " Calls with the same arguments will return the cached value."
                ^ "\n\nCaching is per-request and has Late Static Binding, so subclasses do not share the cache.";
            } );
          ( uaPHPStdLib,
            {
              contexts = [cls; fn; mthd];
              autocomplete = false;
              doc =
                "Ignore this built-in function or class, so the type checker errors if code uses it."
                ^ " This only applies to code in .hhi files by default, but can apply everywhere with `deregister_php_stdlib`.";
            } );
          ( uaAcceptDisposable,
            {
              contexts = [parameter];
              autocomplete = true;
              doc =
                "Allows passing values that implement `IDisposable` or `IAsyncDisposable`."
                ^ " Normally these values cannot be passed to functions."
                ^ "\n\nYou cannot save references to `__AcceptDisposable` parameters, to ensure they are disposed at the end of their using block.";
            } );
          ( uaReturnDisposable,
            {
              contexts = [fn; mthd; lambda];
              autocomplete = true;
              doc =
                "Allows a function/method to return a value that implements `IDisposable` or `IAsyncDisposable`."
                ^ " The function must return a fresh disposable value by either instantiating a class or "
                ^ " returning a value from another method/function marked `__ReturnDisposable`.";
            } );
          ( uaLSB,
            {
              contexts = [staticProperty];
              autocomplete = true;
              doc =
                "Marks this property as implicitly redeclared on all subclasses."
                ^ " This ensures each subclass has its own value for the property.";
            } );
          ( uaSealed,
            {
              contexts = [cls; enumcls; enum];
              autocomplete = true;
              doc =
                "Only the named classes can extend this class or interface."
                ^ " Child classes may still be extended unless they are marked `final`.";
            } );
          ( uaLateInit,
            {
              contexts = [instProperty; staticProperty];
              autocomplete = true;
              doc =
                "Marks a property as late initialized."
                ^ " Normally properties are required to be initialized in the constructor.";
            } );
          ( uaNewable,
            {
              contexts = [typeparam];
              autocomplete = true;
              doc =
                "Ensures the class can be constructed."
                ^ "\n\nThis forbids abstract classes, and ensures that the constructor has a consistent signature."
                ^ " Classes must use `__ConsistentConstruct` or be final.";
            } );
          ( uaEnforceable,
            {
              contexts = [typeconst; typeparam];
              autocomplete = true;
              doc =
                "Ensures that this type is enforceable."
                ^ " Enforceable types can be used with `is` and `as`."
                ^ " This forbids usage of function types and erased (not reified) generics.";
            } );
          ( uaExplicit,
            {
              contexts = [typeparam];
              autocomplete = true;
              doc =
                "Requires callers to explicitly specify this type."
                ^ "\n\nNormally Hack allows generics to be inferred at the call site.";
            } );
          ( uaSoft,
            {
              contexts = [instProperty; parameter; staticProperty; typeparam];
              autocomplete = true;
              doc =
                "A runtime type mismatch on this parameter/property will not throw a TypeError/Error."
                ^ " This is useful for migrating partial code where you're unsure about the type."
                ^ "\n\nThe type checker will ignore this attribute, so your code will still get type checked."
                ^ " If the type is wrong at runtime, a warning will be logged and code execution will continue.";
            } );
          ( uaWarn,
            {
              contexts = [typeparam];
              autocomplete = true;
              doc =
                "Ensures that incorrect reified types are a warning rather than error."
                ^ "\n\nThis is intended to help gradually migrate code to reified types.";
            } );
          ( uaMockClass,
            {
              contexts = [cls];
              autocomplete = false;
              doc =
                "Allows subclasses of final classes and overriding of final methods."
                ^ " This is useful for writing mock classes."
                ^ "\n\nYou cannot use this to subclass `vec`, `keyset`, `dict`, `Vector`, `Map` or `Set`.";
            } );
          ( uaProvenanceSkipFrame,
            {
              contexts = [fn; mthd; lambda];
              autocomplete = false;
              doc =
                "Don't track Hack arrays created by this function."
                ^ " This is useful when migrating code from PHP arrays to Hack arrays.";
            } );
          ( uaDynamicallyCallable,
            {
              contexts = [fn; mthd];
              autocomplete = true;
              doc =
                "Allows this function/method to be called dynamically, based on a string of its name. "
                ^ " HHVM will warn or error (depending on settings) on dynamic calls to functions without this attribute."
                ^ "\n\nSee also `HH\\dynamic_fun()` and `HH\\dynamic_class_meth()`.";
            } );
          ( uaDynamicallyConstructible,
            {
              contexts = [cls];
              autocomplete = true;
              doc =
                "Allows this class to be instantiated dynamically, based on a string of its name."
                ^ " HHVM will warn or error (depending on settings) on dynamic instantiations without this attribute.";
            } );
          ( uaDynamicallyReferenced,
            {
              contexts = [cls];
              autocomplete = true;
              doc =
                "Allows a user to get a pointer to this class from a string of its name."
                ^ " HHVM will warn or error (depending on settings) on dynamic references without this attribute.";
            } );
          ( uaReifiable,
            {
              contexts = [typeconst];
              autocomplete = true;
              doc =
                "Requires this type to be reifiable."
                ^ " This bans PHP arrays (varray and darray).";
            } );
          ( uaNeverInline,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc =
                "Instructs HHVM to never inline this function."
                ^ " Only used for testing HHVM."
                ^ "\n\nSee also `__ALWAYS_INLINE`.";
            } );
          ( uaReified,
            {
              contexts = [];
              autocomplete = false;
              doc =
                "Marks a function as taking reified generics."
                ^ " This is an internal attribute used for byte compilation, and is banned in user code.";
            } );
          ( uaHasReifiedParent,
            {
              contexts = [];
              autocomplete = false;
              doc =
                "Marks a class as extending a class that uses reified generics."
                ^ " This is an internal attribute used for byte compilation, and is banned in user code.";
            } );
          ( uaNoFlatten,
            {
              contexts = [];
              autocomplete = false;
              doc =
                "Instructs hhbbc to never inline this trait into classes that use it."
                ^ " Used for testing hhbbc optimizations.";
            } );
          ( uaNativeData,
            {
              contexts = [cls];
              autocomplete = false;
              doc =
                "Associates this class with a native data type (usually a C++ class)."
                ^ " When instantiating this class, the corresponding native object will also be allocated.";
            } );
          ( uaNonDisjoint,
            {
              contexts = [typeparam];
              autocomplete = true;
              doc =
                "Requires this type parameter to have some overlap with the other `<<__NonDisjoint>>` type parameters."
                ^ "\n\nThis prevents Hack inferring completely unrelated types."
                ^ " For example, this allows the typechecker to warn on `C\\contains(vec[1], \"foo\")`.";
            } );
          ( uaDisableTypecheckerInternal,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc =
                "Disables type checking of a function body or method. This is only useful for debugging typechecker performance."
                ^ " The typechecker will discard the body and immediately report an internal error.";
            } );
          ( uaEnableUnstableFeatures,
            {
              contexts = [file];
              autocomplete = true;
              doc =
                "Enables unstable or preview features of Hack."
                ^ "\n\nUnstable features can only run when HHVM has `Hack.Lang.AllowUnstableFeatures` set. This allows local experimentation without using the feature in production."
                ^ "\n\nWhen a feature enters preview, `__EnableUnstableFeatures` is still required but HHVM will allow the code to run.";
            } );
          ( uaEnumClass,
            {
              contexts = [cls; enumcls];
              autocomplete = false;
              doc =
                "Allows initializing class constants with class instances (not just constant expressions). Used when desugaring `enum class`.";
            } );
          ( uaPolicied,
            {
              contexts = [fn; mthd; instProperty; parameter];
              autocomplete = false;
              doc =
                "Associate a definition with a policy. Used for information flow control, requires `<<file:__EnableUnstableFeatures('ifc')>>`.";
            } );
          ( uaInferFlows,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc =
                "Used for IFC, requires `<<file:__EnableUnstableFeatures('ifc')>>`.";
            } );
          ( uaExternal,
            {
              contexts = [parameter];
              autocomplete = false;
              doc =
                "Used for IFC, requires `<<file:__EnableUnstableFeatures('ifc')>>`.";
            } );
          ( uaCanCall,
            {
              contexts = [parameter];
              autocomplete = false;
              doc =
                "Used for IFC, requires `<<file:__EnableUnstableFeatures('ifc')>>`.";
            } );
          ( uaSupportDynamicType,
            {
              contexts = [fn; cls; mthd; lambda; enumcls];
              autocomplete = false;
              doc =
                "Marks methods and functions that can be called on a receiver of type `dynamic` with `dynamic` arguments. Requires the enable_sound_dynamic_type typechecking flag.";
            } );
          ( uaNoAutoDynamic,
            {
              contexts = [fn; cls; mthd; typealias];
              autocomplete = false;
              doc = "Locally disable implicit pessimisation.";
            } );
          ( uaNoAutoLikes,
            {
              contexts =
                [fn; mthd; staticProperty; instProperty; parameter; lambda];
              autocomplete = false;
              doc = "Locally disable addition of ~ types.";
            } );
          ( uaNoAutoBound,
            {
              contexts = [typeparam];
              autocomplete = false;
              doc = "Locally disable addition of supportdyn<mixed> bound.";
            } );
          ( uaRequireDynamic,
            {
              contexts = [typeparam];
              autocomplete = false;
              doc =
                "Marks this type parameter as required to be `dynamic`. Requires the enable_sound_dynamic_type typechecking flag.";
            } );
          ( uaEnableMethodTraitDiamond,
            {
              contexts = [cls];
              autocomplete = true;
              doc =
                "Allows a trait to be `use`d more than once. "
                ^ "This is useful in large class hierarchies, where you can end up using the same trait on via multiple paths, producing 'diamond inheritance'."
                ^ "\n\nThis requires methods to unambiguous: each method definition must occur in exactly one trait.";
            } );
          ( uaSafeGlobalVariable,
            {
              contexts = [staticProperty];
              autocomplete = false;
              doc =
                "Marks this global variable as safe from mutation."
                ^ " This ensures the global_access_check does NOT raise errors/warnings from writing to this global variable.";
            } );
          ( uaModuleLevelTrait,
            {
              contexts = [cls];
              autocomplete = false;
              doc =
                "Consider the trait to belong to the module where it is defined, "
                ^ "rather than to the module of the class that uses it.";
            } );
          ( uaSoftInternal,
            {
              (* Parameters are for constructor promotion: if someone tries to use it on a
                 parameter without internal, they'll encounter a nast check error *)
              contexts =
                [
                  fn;
                  cls;
                  mthd;
                  instProperty;
                  staticProperty;
                  parameter;
                  enum;
                  enumcls;
                ];
              autocomplete = false;
              doc =
                "Instead of throwing an exception upon a module boundary violation at this symbol, logs a warning instead.";
            } );
          ( uaCrossPackage,
            {
              contexts = [fn; mthd];
              autocomplete = true;
              doc =
                "Enables access to elements from other package(s), requires `<<file:__EnableUnstableFeatures('package')>>`";
            } );
          ( uaStrictSwitch,
            {
              contexts = [fn; mthd];
              autocomplete = true;
              doc =
                "Enables strict switch checking for all switches in function or method.";
            } );
        ])

  (* These are names which are allowed in the systemlib but not in normal programs *)
  let systemlib_map =
    AttributeKinds.(
      SMap.of_list
        [
          ( uaAlwaysInline,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc =
                "Instructs HHVM to always inline this function."
                ^ " Only used for testing HHVM."
                ^ "\n\nSee also `__NEVER_INLINE`.";
            } );
          ( uaIsFoldable,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc =
                "Marks that this function can be constant-folded if all arguments are constants."
                ^ " Used by hhbbc.";
            } );
          ( uaNative,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc =
                "Declares a native function."
                ^ " This declares the signature, the implementation will be in an HHVM extension (usually C++).";
            } );
          ( uaEagerVMSync,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc =
                "Declares that runtime will eagerly sync vm registers for this function.";
            } );
          ( uaOutOnly,
            {
              contexts = [parameter];
              autocomplete = false;
              doc =
                "Declares that an `inout` parameter is written but never read.";
            } );
          ( uaIgnoreReadonlyLocalErrors,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc = "Disables `readonly` compiler checks (systemlib only).";
            } );
          ( uaIgnoreCoeffectLocalErrors,
            {
              contexts = [fn; mthd];
              autocomplete = false;
              doc =
                "Disables context/capability runtime checks (systemlib only).";
            } );
        ])

  let is_reserved name = String.is_prefix name ~prefix:"__"
end

(* Tested before \\-prepending name-canonicalization *)
module SpecialFunctions = struct
  let echo = "echo" (* pseudo-function *)

  let is_special_function =
    let all_special_functions = HashSet.of_list [echo] in
    (fun x -> HashSet.mem all_special_functions x)
end

(* There are a number of functions that are automatically imported into the
 * namespace. The full list can be found in hh_autoimport.ml.
 *)
module AutoimportedFunctions = struct
  let invariant_violation = "\\HH\\invariant_violation"

  let invariant = "\\HH\\invariant"

  let meth_caller = "\\HH\\meth_caller"
end

module SpecialIdents = struct
  let this = "$this"

  let placeholder = "$_"

  let dollardollar = "$$"

  (* Intentionally using an invalid variable name to ensure it's translated *)
  let tmp_var_prefix = "__tmp$"

  let is_tmp_var name =
    String.length name > 6 && String.(sub name ~pos:0 ~len:6 = tmp_var_prefix)

  let assert_tmp_var name = assert (is_tmp_var name)
end

(* PseudoFunctions are functions (or items that are parsed like functions)
 * that are treated like builtins that do not have a public HHI or interface.
 *)
module PseudoFunctions = struct
  let isset = "\\isset"

  let unset = "\\unset"

  let hh_show = "\\hh_show"

  let hh_expect = "\\hh_expect"

  let hh_expect_equivalent = "\\hh_expect_equivalent"

  let hh_show_env = "\\hh_show_env"

  let hh_log_level = "\\hh_log_level"

  let hh_force_solve = "\\hh_force_solve"

  let hh_loop_forever = "\\hh_loop_forever"

  let hh_time = "\\hh_time"

  let echo = "\\echo"

  let empty = "\\empty"

  let exit = "\\exit"

  let die = "\\die"

  let unsafe_cast = "\\HH\\FIXME\\UNSAFE_CAST"

  let unsafe_nonnull_cast = "\\HH\\FIXME\\UNSAFE_NONNULL_CAST"

  let enforced_cast = "\\HH\\FIXME\\ENFORCED_CAST"

  let all_pseudo_functions =
    HashSet.of_list
      [
        isset;
        unset;
        hh_show;
        hh_expect;
        hh_expect_equivalent;
        hh_show_env;
        hh_log_level;
        hh_force_solve;
        hh_loop_forever;
        echo;
        empty;
        exit;
        die;
        unsafe_cast;
        unsafe_nonnull_cast;
      ]

  let is_pseudo_function x = HashSet.mem all_pseudo_functions x
end

module StdlibFunctions = struct
  let is_array = "\\is_array"

  let is_null = "\\is_null"

  let get_class = "\\get_class"

  let array_filter = "\\array_filter"

  let call_user_func = "\\call_user_func"

  let type_structure = "\\HH\\type_structure"

  let array_mark_legacy = "\\HH\\array_mark_legacy"

  let array_unmark_legacy = "\\HH\\array_unmark_legacy"

  let is_any_array = "\\HH\\is_any_array"

  let is_dict_or_darray = "\\HH\\is_dict_or_darray"

  let is_vec_or_varray = "\\HH\\is_vec_or_varray"

  (* All Id funcions that Typing.dispatch_call handles specially *)
  let special_dispatch =
    String.Hash_set.of_list
      ~growth_allowed:false
      [
        SpecialFunctions.echo;
        PseudoFunctions.isset;
        PseudoFunctions.unset;
        type_structure;
        PseudoFunctions.unsafe_cast;
        PseudoFunctions.unsafe_nonnull_cast;
      ]

  let needs_special_dispatch x = Hash_set.mem special_dispatch x
end

module Typehints = struct
  let null = "null"

  let void = "void"

  let resource = "resource"

  let num = "num"

  let arraykey = "arraykey"

  let noreturn = "noreturn"

  let mixed = "mixed"

  let nonnull = "nonnull"

  let this = "this"

  let dynamic = "dynamic"

  let nothing = "nothing"

  let int = "int"

  let bool = "bool"

  let float = "float"

  let string = "string"

  let darray = "darray"

  let varray = "varray"

  let varray_or_darray = "varray_or_darray"

  let vec_or_dict = "vec_or_dict"

  let callable = "callable"

  let wildcard = "_"

  let is_reserved_type_hint =
    let reserved_typehints =
      HashSet.of_list
        [
          null;
          void;
          resource;
          num;
          arraykey;
          noreturn;
          mixed;
          nonnull;
          this;
          dynamic;
          nothing;
          int;
          bool;
          float;
          string;
          darray;
          varray;
          varray_or_darray;
          vec_or_dict;
          callable;
          wildcard;
        ]
    in
    (fun x -> HashSet.mem reserved_typehints x)

  let is_reserved_global_name x =
    String.equal x callable
    || String.equal x Classes.cSelf
    || String.equal x Classes.cParent

  let is_reserved_hh_name x =
    String.equal x void
    || String.equal x noreturn
    || String.equal x int
    || String.equal x bool
    || String.equal x float
    || String.equal x num
    || String.equal x string
    || String.equal x resource
    || String.equal x mixed
    || String.equal x arraykey
    || String.equal x dynamic
    || String.equal x wildcard
    || String.equal x null
    || String.equal x nonnull
    || String.equal x nothing
    || String.equal x this

  let is_namespace_with_reserved_hh_name x =
    let unqualify qualified_name =
      let as_list = Str.split (Str.regexp "\\") qualified_name in
      let as_list = List.filter as_list ~f:(fun s -> not (String.equal s "")) in
      match List.rev as_list with
      | name :: qualifiers -> (List.rev qualifiers, name)
      | [] -> ([], qualified_name)
    in
    let is_HH qualifier =
      match qualifier with
      | [qual] -> String.equal qual "HH"
      | _ -> false
    in
    let (qualifier, name) = unqualify x in
    name |> is_reserved_hh_name
    && (not (List.is_empty qualifier))
    && not (qualifier |> is_HH)
end

module PseudoConsts = struct
  let g__LINE__ = "\\__LINE__"

  let g__CLASS__ = "\\__CLASS__"

  let g__TRAIT__ = "\\__TRAIT__"

  let g__FILE__ = "\\__FILE__"

  let g__DIR__ = "\\__DIR__"

  let g__FUNCTION__ = "\\__FUNCTION__"

  let g__METHOD__ = "\\__METHOD__"

  let g__NAMESPACE__ = "\\__NAMESPACE__"

  let g__COMPILER_FRONTEND__ = "\\__COMPILER_FRONTEND__"

  let g__FUNCTION_CREDENTIAL__ = "\\__FUNCTION_CREDENTIAL__"

  (* exit and die are not pseudo consts, but they are currently parsed as such.
   * Would be more correct to parse them as special statements like return
   *)
  let exit = "\\exit"

  let die = "\\die"

  let all_pseudo_consts =
    HashSet.of_list
      [
        g__LINE__;
        g__CLASS__;
        g__TRAIT__;
        g__FILE__;
        g__DIR__;
        g__FUNCTION__;
        g__METHOD__;
        g__NAMESPACE__;
        g__COMPILER_FRONTEND__;
        g__FUNCTION_CREDENTIAL__;
        exit;
        die;
      ]

  let is_pseudo_const x = HashSet.mem all_pseudo_consts x
end

module FB = struct
  let cEnum = "\\Enum"

  let tInner = "TInner"

  let idx = "\\HH\\idx"

  let cTypeStructure = "\\HH\\TypeStructure"
end

module HH = struct
  let memoizeOption = "\\HH\\MemoizeOption"

  let contains = "\\HH\\Lib\\C\\contains"

  let contains_key = "\\HH\\Lib\\C\\contains_key"

  module FIXME = struct
    let tTanyMarker = "\\HH\\FIXME\\TANY_MARKER"

    let tPoisonMarker = "\\HH\\FIXME\\POISON_MARKER"
  end
end

module Shapes = struct
  let cShapes = "\\HH\\Shapes"

  let cReadonlyShapes = "\\HH\\Readonly\\Shapes"

  let idx = "idx"

  let at = "at"

  let keyExists = "keyExists"

  let removeKey = "removeKey"

  let toArray = "toArray"

  let toDict = "toDict"
end

module Hips = struct
  let inspect = "\\inspect"
end

module Superglobals = struct
  let globals = "$GLOBALS"

  let is_superglobal =
    let superglobals =
      HashSet.of_list
        [
          "$_SERVER";
          "$_GET";
          "$_POST";
          "$_FILES";
          "$_COOKIE";
          "$_REQUEST";
          "$_ENV";
        ]
    in
    (fun x -> HashSet.mem superglobals x)
end

module Regex = struct
  let tPattern = "\\HH\\Lib\\Regex\\Pattern"
end

(* These are functions treated by the emitter specially. They are not
 * autoimported (see hh_autoimport.ml) nor are they consider PseudoFunctions
 * so they can be overridden by namespacing (at least currently)
 *)
module EmitterSpecialFunctions = struct
  let eval = "\\eval"

  let set_frame_metadata = "\\HH\\set_frame_metadata"

  let systemlib_reified_generics = "\\__systemlib_reified_generics"
end

module XHP = struct
  let pcdata = "pcdata"

  let any = "any"

  let empty = "empty"

  let is_reserved name =
    String.equal name pcdata || String.equal name any || String.equal name empty

  let is_xhp_category name = String.is_prefix name ~prefix:"%"
end

(* This should be a subset of rust_parser_errors::UnstableFeatures that is relevant
 * to the typechecker *)
module UnstableFeatures = struct
  let coeffects_provisional = "coeffects_provisional"

  let ifc = "ifc"

  let readonly = "readonly"

  let expression_trees = "expression_trees"

  let modules = "modules"
end

module Coeffects = struct
  let capability = "$#capability"

  let local_capability = "$#local_capability"

  let contexts = "\\HH\\Contexts"

  let unsafe_contexts = contexts ^ "\\Unsafe"

  let generated_generic_prefix = "T/"

  let is_generated_generic = String.is_prefix ~prefix:generated_generic_prefix

  (** "T/[ctx $foo]" to "ctx $foo". *)
  let unwrap_generated_generic name =
    name
    |> String.chop_prefix_if_exists ~prefix:"T/["
    |> String.chop_suffix_if_exists ~suffix:"]"
end

module Readonly = struct
  let prefix = "\\HH\\Readonly\\"

  let idx = "\\HH\\idx_readonly"

  let as_mut = prefix ^ "as_mut"
end

module Modules = struct
  let default = "default"
end

module Capabilities = struct
  let defaults = Coeffects.contexts ^ "\\defaults"

  let write_props = Coeffects.contexts ^ "\\write_props"

  let prefix = "\\HH\\Capabilities\\"

  let writeProperty = prefix ^ "WriteProperty"

  let accessGlobals = prefix ^ "AccessGlobals"

  let readGlobals = prefix ^ "ReadGlobals"

  let systemLocal = prefix ^ "SystemLocal"

  let implicitPolicy = prefix ^ "ImplicitPolicy"

  let implicitPolicyLocal = prefix ^ "ImplicitPolicyLocal"

  let io = prefix ^ "IO"

  let rx = prefix ^ "Rx"

  let rxLocal = rx ^ "Local"
end

module ExpressionTrees = struct
  let makeTree = "makeTree"

  let intType = "intType"

  let floatType = "floatType"

  let boolType = "boolType"

  let stringType = "stringType"

  let nullType = "nullType"

  let voidType = "voidType"

  let symbolType = "symbolType"

  let visitInt = "visitInt"

  let visitFloat = "visitFloat"

  let visitBool = "visitBool"

  let visitString = "visitString"

  let visitNull = "visitNull"

  let visitBinop = "visitBinop"

  let visitUnop = "visitUnop"

  let visitLocal = "visitLocal"

  let visitLambda = "visitLambda"

  let visitGlobalFunction = "visitGlobalFunction"

  let visitStaticMethod = "visitStaticMethod"

  let visitCall = "visitCall"

  let visitAssign = "visitAssign"

  let visitTernary = "visitTernary"

  let visitIf = "visitIf"

  let visitWhile = "visitWhile"

  let visitReturn = "visitReturn"

  let visitFor = "visitFor"

  let visitBreak = "visitBreak"

  let visitContinue = "visitContinue"

  let splice = "splice"

  let dollardollarTmpVar = "$0dollardollar"
end
