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

  let cThrowable = "\\Throwable"

  let cStdClass = "\\stdClass"

  let cDateTime = "\\DateTime"

  let cDateTimeImmutable = "\\DateTimeImmutable"

  let cAsyncIterator = "\\HH\\AsyncIterator"

  let cAsyncKeyedIterator = "\\HH\\AsyncKeyedIterator"

  let cStringish = "\\Stringish"

  let cXHPChild = "\\XHPChild"

  let cIMemoizeParam = "\\HH\\IMemoizeParam"

  let cClassname = "\\HH\\classname"

  let cTypename = "\\HH\\typename"

  let cIDisposable = "\\IDisposable"

  let cIAsyncDisposable = "\\IAsyncDisposable"
end

module Collections = struct
  (* concrete classes *)
  let cVector = "\\HH\\Vector"

  let cImmVector = "\\HH\\ImmVector"

  let cSet = "\\HH\\Set"

  let cImmSet = "\\HH\\ImmSet"

  let cMap = "\\HH\\Map"

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

  let cDict = "\\HH\\dict"

  let cVec = "\\HH\\vec"

  let cKeyset = "\\HH\\keyset"
end

module Members = struct
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
    String_utils.string_starts_with s ":data-"
    || String_utils.string_starts_with s ":aria-"
end

module AttributeKinds = struct
  let cls = "\\HH\\ClassAttribute"

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

  let plain_english_map =
    List.fold_left
      ~init:SMap.empty
      ~f:(fun acc (k, v) -> SMap.add k v acc)
      [
        (cls, "a class");
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
      ]
end

module UserAttributes = struct
  let uaOverride = "__Override"

  let uaConsistentConstruct = "__ConsistentConstruct"

  let uaConst = "__Const"

  let uaDeprecated = "__Deprecated"

  let uaEntryPoint = "__EntryPoint"

  let uaMemoize = "__Memoize"

  let uaMemoizeLSB = "__MemoizeLSB"

  let uaPHPStdLib = "__PHPStdLib"

  let uaHipHopSpecific = "__HipHopSpecific"

  let uaAcceptDisposable = "__AcceptDisposable"

  let uaReturnDisposable = "__ReturnDisposable"

  let uaPure = "__Pure"

  let uaCipp = "__Cipp"

  let uaCippLocal = "__CippLocal"

  let uaCippGlobal = "__CippGlobal"

  let uaReactive = "__Rx"

  let uaLocalReactive = "__RxLocal"

  let uaShallowReactive = "__RxShallow"

  let uaMutable = "__Mutable"

  let uaMutableReturn = "__MutableReturn"

  let uaOnlyRxIfImpl = "__OnlyRxIfImpl"

  let uaLSB = "__LSB"

  let uaAtMostRxAsFunc = "__AtMostRxAsFunc"

  let uaAtMostRxAsArgs = "__AtMostRxAsArgs"

  let uaSealed = "__Sealed"

  let uaReturnsVoidToRx = "__ReturnsVoidToRx"

  let uaMaybeMutable = "__MaybeMutable"

  let uaLateInit = "__LateInit"

  let uaOwnedMutable = "__OwnedMutable"

  let uaNonRx = "__NonRx"

  let uaNewable = "__Newable"

  let uaEnforceable = "__Enforceable"

  let uaExplicit = "__Explicit"

  let uaSoft = "__Soft"

  let uaWarn = "__Warn"

  let uaMockClass = "__MockClass"

  let uaProvenanceSkipFrame = "__ProvenanceSkipFrame"

  let uaDynamicallyCallable = "__DynamicallyCallable"

  let uaDynamicallyConstructible = "__DynamicallyConstructible"

  let uaReifiable = "__Reifiable"

  let uaNeverInline = "__NEVER_INLINE"

  let uaDisableTypecheckerInternal = "__DisableTypecheckerInternal"

  let uaHasTopLevelCode = "__HasTopLevelCode"

  let uaIsFoldable = "__IsFoldable"

  let uaNative = "__Native"

  let uaOutOnly = "__OutOnly"

  let uaAlwaysInline = "__ALWAYS_INLINE"

  let uaPu = "__Pu"

  let uaEnableUnstableFeatures = "__EnableUnstableFeatures"

  let as_map =
    AttributeKinds.(
      SMap.of_list
        [
          (uaOverride, [mthd]);
          (uaConsistentConstruct, [cls]);
          (uaConst, [cls; instProperty; parameter; staticProperty]);
          (uaDeprecated, [fn; mthd]);
          (uaEntryPoint, [fn]);
          (uaMemoize, [fn; mthd]);
          (uaMemoizeLSB, [fn; mthd]);
          (uaPHPStdLib, [cls; fn; mthd]);
          (uaHipHopSpecific, [cls]);
          (uaAcceptDisposable, [parameter]);
          (uaReturnDisposable, [fn; mthd]);
          (uaPure, [fn; mthd]);
          (uaCipp, [fn; mthd]);
          (uaCippLocal, [fn; mthd]);
          (uaCippGlobal, [fn; mthd]);
          (uaReactive, [fn; mthd]);
          (uaLocalReactive, [fn; mthd]);
          (uaMutable, [mthd; parameter]);
          (uaMutableReturn, [fn; mthd]);
          (uaShallowReactive, [fn; mthd]);
          (uaOnlyRxIfImpl, [parameter; mthd]);
          (uaLSB, [staticProperty]);
          (uaSealed, [cls]);
          (uaReturnsVoidToRx, [fn; mthd]);
          (uaMaybeMutable, [mthd; parameter]);
          (uaLateInit, [instProperty; parameter; staticProperty]);
          (uaAtMostRxAsFunc, [parameter]);
          (uaAtMostRxAsArgs, [fn; mthd]);
          (uaOwnedMutable, [parameter]);
          (uaNonRx, [fn; mthd]);
          (uaNewable, [typeparam]);
          (uaEnforceable, [typeconst; typeparam]);
          (uaExplicit, [typeparam]);
          (uaSoft, [instProperty; parameter; staticProperty; typeparam]);
          (uaWarn, [typeparam]);
          (uaMockClass, [cls]);
          (uaProvenanceSkipFrame, [fn; mthd]);
          (uaDynamicallyCallable, [fn; mthd]);
          (uaDynamicallyConstructible, [cls]);
          (uaReifiable, [typeconst]);
          (uaNeverInline, [fn; mthd]);
          (uaDisableTypecheckerInternal, [fn; mthd]);
          (uaPu, [cls]);
          (uaEnableUnstableFeatures, [file]);
        ])

  (* These are names which are allowed in the systemlib but not in normal programs *)
  let systemlib_map =
    AttributeKinds.(
      SMap.of_list
        [
          (uaAlwaysInline, [fn; mthd]);
          (uaIsFoldable, [fn; mthd]);
          (uaNative, [fn; mthd]);
          (uaOutOnly, [parameter]);
        ])

  let is_reserved name = String.is_prefix name ~prefix:"__"
end

(* Tested before \\-prepending name-canonicalization *)
module SpecialFunctions = struct
  let tuple = "tuple" (* pseudo-function *)

  let echo = "echo" (* pseudo-function *)

  let assert_ = "assert"

  let hhas_adata = "__hhas_adata"

  let is_special_function =
    let all_special_functions =
      HashSet.of_list [tuple; echo; assert_; hhas_adata]
    in
    (fun x -> HashSet.mem all_special_functions x)
end

(* There are a number of functions that are automatically imported into the
 * namespace. The full list can be found in hh_autoimport.ml.
 *)
module AutoimportedFunctions = struct
  let invariant_violation = "\\HH\\invariant_violation"

  let invariant = "\\HH\\invariant"

  let fun_ = "\\HH\\fun"

  let inst_meth = "\\HH\\inst_meth"

  let class_meth = "\\HH\\class_meth"

  let meth_caller = "\\HH\\meth_caller"
end

module SpecialIdents = struct
  let this = "$this"

  let placeholder = "$_"

  let dollardollar = "$$"

  (* Intentionally using an invalid variable name to ensure it's translated *)
  let tmp_var_prefix = "__tmp$"

  let is_tmp_var name =
    String.length name > 6 && String.(sub name 0 6 = tmp_var_prefix)

  let assert_tmp_var name = assert (is_tmp_var name)
end

(* PseudoFunctions are functions (or items that are parsed like functions)
 * that are treated like builtins that do not have a public HHI or interface.
 *)
module PseudoFunctions = struct
  let isset = "\\isset"

  let unset = "\\unset"

  let hh_show = "\\hh_show"

  let hh_show_env = "\\hh_show_env"

  let hh_log_level = "\\hh_log_level"

  let hh_force_solve = "\\hh_force_solve"

  let hh_loop_forever = "\\hh_loop_forever"

  let assert_ = "\\assert"

  let echo = "\\echo"

  let empty = "\\empty"

  let exit = "\\exit"

  let die = "\\die"

  let all_pseudo_functions =
    HashSet.of_list
      [
        isset;
        unset;
        hh_show;
        hh_show_env;
        hh_log_level;
        hh_force_solve;
        hh_loop_forever;
        assert_;
        echo;
        empty;
        exit;
        die;
      ]

  let is_pseudo_function x = HashSet.mem all_pseudo_functions x
end

module StdlibFunctions = struct
  let is_null = "\\is_null"

  let get_class = "\\get_class"

  let array_filter = "\\array_filter"

  let array_map = "\\array_map"

  let call_user_func = "\\call_user_func"

  let type_structure = "\\HH\\type_structure"

  let array_mark_legacy = "\\HH\\array_mark_legacy"

  let array_unmark_legacy = "\\HH\\array_unmark_legacy"

  let is_php_array = "\\HH\\is_php_array"

  let is_any_array = "\\HH\\is_any_array"

  let is_dict_or_darray = "\\HH\\is_dict_or_darray"

  let is_vec_or_varray = "\\HH\\is_vec_or_varray"
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

  let array = "array"

  let darray = "darray"

  let varray = "varray"

  let varray_or_darray = "varray_or_darray"

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
          array;
          darray;
          varray;
          varray_or_darray;
          callable;
          wildcard;
        ]
    in
    (fun x -> HashSet.mem reserved_typehints x)

  let is_reserved_global_name x =
    String.equal x array
    || String.equal x callable
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
    || String.equal x array
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
      let as_list = List.filter as_list ~f:(fun s -> not (phys_equal s "")) in
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

  let cIncorrectType = "\\HH\\INCORRECT_TYPE"
end

module HH = struct
  let contains = "\\HH\\Lib\\C\\contains"

  let contains_key = "\\HH\\Lib\\C\\contains_key"
end

module Rx = struct
  let freeze = "\\HH\\Rx\\freeze"

  let mutable_ = "\\HH\\Rx\\mutable"

  let cTraversable = "\\HH\\Rx\\Traversable"

  let is_enabled v =
    String.equal v "\\HH\\Rx\\IS_ENABLED" || String.equal v "\\Rx\\IS_ENABLED"

  let cKeyedTraversable = "\\HH\\Rx\\KeyedTraversable"

  let cAsyncIterator = "\\HH\\Rx\\AsyncIterator"

  let move = "\\HH\\Rx\\move"

  let hPure = "Pure"

  let hRx = "Rx"

  let hRxShallow = "RxShallow"

  let hRxLocal = "RxLocal"

  let hMutable = "Mutable"

  let hMaybeMutable = "MaybeMutable"

  let hOwnedMutable = "OwnedMutable"

  let is_reactive_typehint =
    let reactive_typehints =
      [hPure; hRx; hRxShallow; hRxLocal; hMutable; hMaybeMutable; hOwnedMutable]
    in
    fun name ->
      List.exists reactive_typehints ~f:(fun th -> String.equal th name)
end

module Shapes = struct
  let cShapes = "\\HH\\Shapes"

  let idx = "idx"

  let at = "at"

  let keyExists = "keyExists"

  let removeKey = "removeKey"

  let toArray = "toArray"

  let toDict = "toDict"
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

module PPLFunctions = struct
  let all_reserved =
    HashSet.of_list
      [
        "sample";
        "\\sample";
        "factor";
        "\\factor";
        "observe";
        "\\observe";
        "condition";
        "\\condition";
        "sample_model";
        "\\sample_model";
      ]

  let is_reserved name = HashSet.mem all_reserved name
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
end

module PocketUniverses = struct
  let members = "Members"
end

module XHP = struct
  let pcdata = "pcdata"

  let any = "any"

  let empty = "empty"

  let is_reserved name =
    String.equal name pcdata || String.equal name any || String.equal name empty

  let is_xhp_category name = String_utils.string_starts_with name "%"
end

(* This should be a subset of rust_parser_errors::UnstableFeatures that is relevant
 * to the typechecker *)
module UnstableFeatures = struct
  let coeffects_provisional = "coeffects_provisional"
end
