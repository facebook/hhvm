(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(** Module consisting of the special names known to the typechecker *)

module Classes = struct

  let cParent = "parent"
  let cStatic = "static"
  let cSelf   = "self"
  let cUnknown = "\\*Unknown*" (* Used for dynamic classnames, e.g. new $foo(); *)

  let cAwaitable = "\\Awaitable"
  let cGenerator = "\\Generator"
  let cAsyncGenerator = "\\AsyncGenerator"
  let is_format_string x = match x with
    "\\FormatString" | "\\HH\\FormatString" -> true
    | _ -> false

  let cHH_BuiltinEnum = "\\HH\\BuiltinEnum"

  let cThrowable= "\\Throwable"
  let cStdClass = "\\stdClass"
  let cDateTime = "\\DateTime"
  let cDateTimeImmutable = "\\DateTimeImmutable"

  let cAsyncIterator = "\\AsyncIterator"
  let cAsyncKeyedIterator = "\\AsyncKeyedIterator"

  let cStringish = "\\Stringish"
  let cXHPChild = "\\XHPChild"
  let cIMemoizeParam = "\\IMemoizeParam"
  let cClassname = "\\classname"
  let cTypename = "\\typename"

  let cIDisposable = "\\IDisposable"
  let cIAsyncDisposable = "\\IAsyncDisposable"
end

module Collections = struct

  (* concrete classes *)
  let cVector    = "\\Vector"
  let cImmVector = "\\ImmVector"
  let cSet       = "\\Set"
  let cImmSet    = "\\ImmSet"
  let cMap       = "\\Map"
  let cStableMap = "\\StableMap"
  let cImmMap    = "\\ImmMap"
  let cPair      = "\\Pair"

  (* interfaces *)
  let cContainer        = "\\Container"
  let cKeyedContainer   = "\\KeyedContainer"
  let cTraversable      = "\\Traversable"
  let cKeyedTraversable = "\\KeyedTraversable"
  let cCollection       = "\\Collection"

  let cConstVector      = "\\ConstVector"
  let cConstMap         = "\\ConstMap"
  let cConstCollection  = "\\ConstCollection"
  let cDict             = "\\dict"
  let cVec              = "\\vec"
  let cKeyset           = "\\keyset"

end

module Members = struct

  let mClass       = "class"

  let __construct  = "__construct"
  let __destruct   = "__destruct"
  let __call       = "__call"
  let __callStatic = "__callStatic"
  let __clone      = "__clone"
  let __debugInfo  = "__debugInfo"
  let __dispose    = "__dispose"
  let __disposeAsync = "__disposeAsync"
  let __get        = "__get"
  let __invoke     = "__invoke"
  let __isset      = "__isset"
  let __set        = "__set"
  let __set_state  = "__set_state"
  let __sleep      = "__sleep"
  let __toString   = "__toString"
  let __unset      = "__unset"
  let __wakeup     = "__wakeup"

  let as_set = List.fold_right ~f:SSet.add ~init:SSet.empty
    [
      __construct; __destruct; __call; __callStatic; __clone; __debugInfo;
      __dispose; __disposeAsync;
      __get; __invoke; __isset; __set; __set_state; __sleep; __toString;
      __unset; __wakeup;
    ]
  let as_lowercase_set = SSet.map String.lowercase as_set

  (* Any data- or aria- attribute is always valid, even if it is not declared
   * for a given XHP element *)
  let is_special_xhp_attribute s =
    (String_utils.string_starts_with s ":data-") ||
    (String_utils.string_starts_with s ":aria-")
end

module UserAttributes = struct

  let uaOverride            = "__Override"
  let uaConsistentConstruct = "__ConsistentConstruct"
  let uaConst               = "__Const"
  let uaDeprecated          = "__Deprecated"
  let uaEntryPoint          = "__EntryPoint"
  let uaMemoize             = "__Memoize"
  let uaMemoizeLSB          = "__MemoizeLSB"
  let uaPHPStdLib           = "__PHPStdLib"
  let uaHipHopSpecific      = "__HipHopSpecific"
  let uaAcceptDisposable    = "__AcceptDisposable"
  let uaReturnDisposable    = "__ReturnDisposable"
  let uaReactive            = "__Rx"
  let uaLocalReactive       = "__RxLocal"
  let uaShallowReactive     = "__RxShallow"
  let uaMutable             = "__Mutable"
  let uaMutableReturn       = "__MutableReturn"
  let uaOptionalDestruct    = "__OptionalDestruct"
  let uaOnlyRxIfImpl        = "__OnlyRxIfImpl"
  let uaProbabilisticModel  = "__PPL"
  let uaLSB                 = "__LSB"
  let uaAtMostRxAsFunc      = "__AtMostRxAsFunc"
  let uaAtMostRxAsArgs      = "__AtMostRxAsArgs"
  let uaSealed              = "__Sealed"
  let uaReturnsVoidToRx     = "__ReturnsVoidToRx"
  let uaMaybeMutable        = "__MaybeMutable"
  let uaLateInit            = "__LateInit"
  let uaSoftLateInit        = "__SoftLateInit"
  let uaOwnedMutable        = "__OwnedMutable"
  let uaNonRx               = "__NonRx"
  let uaNewable             = "__Newable"
  let uaEnforceable         = "__Enforceable"
  let uaSoft                = "__Soft"
  let uaWarn                = "__Warn"
  let uaMockClass           = "__MockClass"

  let as_set = List.fold_right ~f:SSet.add ~init:SSet.empty
    [
      uaOverride;
      uaConsistentConstruct;
      uaConst;
      uaDeprecated;
      uaEntryPoint;
      uaMemoize;
      uaMemoizeLSB;
      uaPHPStdLib;
      uaHipHopSpecific;
      uaAcceptDisposable;
      uaReturnDisposable;
      uaReactive;
      uaLocalReactive;
      uaMutable;
      uaMutableReturn;
      uaShallowReactive;
      uaOptionalDestruct;
      uaOnlyRxIfImpl;
      uaProbabilisticModel;
      uaLSB;
      uaSealed;
      uaReturnsVoidToRx;
      uaMaybeMutable;
      uaLateInit;
      uaSoftLateInit;
      uaAtMostRxAsFunc;
      uaAtMostRxAsArgs;
      uaOwnedMutable;
      uaNonRx;
      uaNewable;
      uaEnforceable;
      uaSoft;
      uaWarn;
      uaMockClass;
    ]
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
    List.fold_left ~init:SMap.empty ~f:(fun acc (k, v) -> SMap.add k v acc)
      [ (cls, "a class")
      ; (enum, "an enum")
      ; (typealias, "a typealias")
      ; (fn, "a function")
      ; (mthd, "a method")
      ; (instProperty, "an instance property")
      ; (staticProperty, "a static property")
      ; (parameter, "a parameter")
      ; (typeparam, "a type parameter")
      ; (file, "a file")
      ; (typeconst, "a type constant")
      ]
end

(* Tested before \\-prepending name-canonicalization *)
module SpecialFunctions = struct

  let tuple          = "tuple"          (* pseudo-function *)
  let echo           = "echo"           (* pseudo-function *)
  let assert_        = "assert"

  let invariant      = "invariant"
  let invariant_violation = "invariant_violation"

  let fun_           = "fun"
  let inst_meth      = "inst_meth"
  let class_meth     = "class_meth"
  let meth_caller    = "meth_caller"

  let call_user_func = "call_user_func"
  let autoload       = "__autoload"
  let clone          = "__clone"
end

module SpecialIdents = struct

  let this = "$this"
  let placeholder = "$_"
  let dollardollar = "$$"

  (* Intentionally using an invalid variable name to ensure it's translated *)
  let tmp_var_prefix = "__tmp$"

  let is_tmp_var name =
    String.length name > 6 &&
    String.sub name 0 6 = tmp_var_prefix

  let assert_tmp_var name =
    assert (is_tmp_var name)

end

module PseudoFunctions = struct

  let empty = "\\empty"
  let isset = "\\isset"
  let unset = "\\unset"
  let hh_show = "\\hh_show"
  let hh_show_env = "\\hh_show_env"
  let hh_log_level = "\\hh_log_level"
  let hh_loop_forever = "\\hh_loop_forever"

  let all_pseudo_functions = [
    empty;
    isset;
    unset;
    hh_show;
    hh_show_env;
    hh_log_level;
    hh_loop_forever;
  ]

end

module StdlibFunctions = struct

  let is_array    = "\\is_array"
  let is_null     = "\\is_null"

  let get_class = "\\get_class"

  let array_filter = "\\array_filter"
  let array_map = "\\array_map"

  let type_structure = "\\type_structure"
end

module Typehints = struct

  let null     = "null"
  let void     = "void"
  let resource = "resource"
  let num      = "num"
  let arraykey = "arraykey"
  let noreturn = "noreturn"
  let mixed    = "mixed"
  let nonnull  = "nonnull"
  let this     = "this"
  let dynamic  = "dynamic"
  let nothing  = "nothing"

  let int     = "int"
  let bool    = "bool"
  let float   = "float"
  let string  = "string"
  let array   = "array"
  let darray  = "darray"
  let varray  = "varray"
  let varray_or_darray = "varray_or_darray"
  let integer = "integer"
  let boolean = "boolean"
  let double  = "double"
  let real    = "real"
  let callable = "callable"

  let object_cast = "object"
  let unset_cast = "unset"

  let wildcard = "_"

  let is_reserved_global_name x =
    let x = String.lowercase x in
    x = array    || x = callable || x = Classes.cSelf || x = Classes.cParent

  let is_reserved_hh_name x =
    let x = String.lowercase x in
    x = void     || x = noreturn || x = int      || x = bool     || x = float ||
    x = num      || x = string   || x = resource || x = mixed    || x = array ||
    x = arraykey || x = integer  || x = boolean  || x = double   || x = real  ||
    x = dynamic  || x = wildcard || x = nonnull  || x = nothing

  let is_namespace_with_reserved_hh_name x =
    let unqualify qualified_name =
      let as_list = Str.split (Str.regexp "\\") qualified_name in
      let as_list = List.filter as_list ~f:(fun s -> not (phys_equal s "")) in
      match List.rev as_list with
      | name :: qualifiers -> List.rev qualifiers, name
      | [] -> [], qualified_name in
    let is_HH qualifier =
      match qualifier with
      | [qual] -> qual = "HH"
      | _ -> false in
    let qualifier, name = unqualify x in
    name |> is_reserved_hh_name
    && not (List.is_empty qualifier)
    && not (qualifier |> is_HH)

end

module PseudoConsts = struct

  let g__LINE__      = "\\__LINE__"
  let g__CLASS__     = "\\__CLASS__"
  let g__TRAIT__     = "\\__TRAIT__"
  let g__FILE__      = "\\__FILE__"
  let g__DIR__       = "\\__DIR__"
  let g__FUNCTION__  = "\\__FUNCTION__"
  let g__METHOD__    = "\\__METHOD__"
  let g__NAMESPACE__ = "\\__NAMESPACE__"
  let g__COMPILER_FRONTEND__ = "\\__COMPILER_FRONTEND__"

  let all_pseudo_consts = [
    g__LINE__; g__CLASS__; g__TRAIT__; g__FILE__; g__DIR__;
    g__FUNCTION__; g__METHOD__; g__NAMESPACE__; g__COMPILER_FRONTEND__
  ]
  let is_pseudo_const =
    let h = HashSet.create 23 in
    List.iter all_pseudo_consts (HashSet.add h);
    fun x -> HashSet.mem h x

end

module FB = struct

  let cEnum                  = "\\Enum"
  let cUncheckedEnum         = "\\UncheckedEnum"

  let fgena                  = "\\gena"
  let fgenva                 = "\\genva"
  let fgen_array_rec         = "\\gen_array_rec"

  let idx                    = "\\idx"

  let cTypeStructure         = "\\TypeStructure"

end

module HH = struct

  let asio_va                = "\\HH\\Asio\\va"
  let lib_tuple_from_async   = "\\HH\\Lib\\Tuple\\from_async"
  let lib_tuple_gen          = "\\HH\\Lib\\Tuple\\gen"

  let contains               = "\\HH\\Lib\\C\\contains"
  let contains_key           = "\\HH\\Lib\\C\\contains_key"

end

module Rx = struct
  let freeze = "\\HH\\Rx\\freeze"
  let mutable_ = "\\HH\\Rx\\mutable"
  let cTraversable = "\\HH\\Rx\\Traversable"
  let is_enabled = "\\HH\\Rx\\IS_ENABLED"
  let cKeyedTraversable = "\\HH\\Rx\\KeyedTraversable"
  let cAsyncIterator = "\\HH\\Rx\\AsyncIterator"
  let move = "\\HH\\Rx\\move"
end

module Shapes = struct
  let cShapes                = "\\Shapes"
  let idx                    = "idx"
  let keyExists              = "keyExists"
  let removeKey              = "removeKey"
  let toArray                = "toArray"
  let toDict                 = "toDict"
end

module Superglobals = struct
  let globals = "$GLOBALS"

  let all_superglobals =
    [globals ; "$_SERVER"; "$_GET"; "$_POST"; "$_FILES";
     "$_COOKIE"; "$_SESSION"; "$_REQUEST"; "$_ENV"
    ]

  let is_superglobal =
    let h = HashSet.create 23 in
    List.iter all_superglobals (HashSet.add h);
    fun x -> HashSet.mem h x
end

module PPLFunctions = struct
  let all_reserved =
    [ "sample"; "\\sample"; "factor"; "\\factor";
      "observe"; "\\observe"; "condition"; "\\condition";
      "sample_model"; "\\sample_model"; "sampleiid"; "\\sampleiid";
      "observeiid"; "\\observeiid";
    ]

  let is_reserved =
    let h = HashSet.create 23 in
    List.iter all_reserved (HashSet.add h);
    fun name -> HashSet.mem h name
end

module Regex = struct
  let tPattern = "\\HH\\Lib\\Regex\\Pattern"
end
