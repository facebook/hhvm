(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

(** Module consisting of the special names known to the typechecker *)

module Classes = struct

  let cParent = "parent"
  let cStatic = "static"
  let cSelf   = "self"
  let cUnknown = "\\*Unknown*" (* Used for dynamic classnames, e.g. new $foo(); *)

  let cAwaitable = "\\Awaitable"
  let cWaitHandle = "\\WaitHandle"
  let cWaitableWaitHandle = "\\WaitableWaitHandle"
  let cGenerator = "\\Generator"
  let cAsyncGenerator = "\\AsyncGenerator"
  let cFormatString = "\\FormatString" (* deprecated - defined in user code *)
  let cHackFormatString = "\\HH\\FormatString" (* Same thing, but in core HHI *)
  let is_format_string x = match x with
    "\\FormatString" | "\\HH\\FormatString" -> true
    | _ -> false

  let cHH_BuiltinEnum = "\\HH\\BuiltinEnum"

  let cException = "\\Exception"
  let cStdClass = "\\stdClass"
  let cDateTime = "\\DateTime"

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
  let cIterator         = "\\Iterator"
  let cKeyedIterator    = "\\KeyedIterator"
  let cContainer        = "\\Container"
  let cKeyedContainer   = "\\KeyedContainer"
  let cTraversable      = "\\Traversable"
  let cKeyedTraversable = "\\KeyedTraversable"
  let cIterable         = "\\Iterable"
  let cKeyedIterable    = "\\KeyedIterable"
  let cIndexish         = "\\Indexish"

  let cCollection       = "\\Collection"
  let cConstVector      = "\\ConstVector"
  let cConstMap         = "\\ConstMap"
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

  (* Any data- or aria- attribute is always valid, even if it is not declared
   * for a given XHP element *)
  let is_special_xhp_attribute s =
    (String_utils.string_starts_with s ":data-") ||
    (String_utils.string_starts_with s ":aria-")
end

module UserAttributes = struct

  let uaOverride            = "__Override"
  let uaConsistentConstruct = "__ConsistentConstruct"
  let uaUnsafeConstruct     = "__UNSAFE_Construct"
  let uaDeprecated          = "__Deprecated"
  let uaMemoize             = "__Memoize"
  let uaPHPStdLib           = "__PHPStdLib"

  let as_set : SSet.t =
    let s = SSet.empty in
    let s = SSet.add uaOverride s in
    let s = SSet.add uaConsistentConstruct s in
    let s = SSet.add uaUnsafeConstruct s in
    let s = SSet.add uaDeprecated s in
    let s = SSet.add uaMemoize s in
    let s = SSet.add uaPHPStdLib s in
    s

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
end

module SpecialIdents = struct

  let this = "$this"
  let placeholder = "$_"
  let dollardollar = "$$"

end

module PseudoFunctions = struct

  let empty = "\\empty"
  let isset = "\\isset"
  let unset = "\\unset"
  let exit_ = "\\exit"
  let die = "\\die"
  let hh_show = "\\hh_show"
  let hh_show_env = "\\hh_show_env"
  let hh_log_level = "\\hh_log_level"

end

module StdlibFunctions = struct

  let is_int      = "\\is_int"
  let is_bool     = "\\is_bool"
  let is_array    = "\\is_array"
  let is_vec      = "\\is_vec"
  let is_dict     = "\\is_dict"
  let is_keyset   = "\\is_keyset"
  let is_float    = "\\is_float"
  let is_string   = "\\is_string"
  let is_null     = "\\is_null"
  let is_resource = "\\is_resource"

  let get_class = "\\get_class"
  let get_called_class = "\\get_called_class" (* treated as static::class *)

  let array_filter = "\\array_filter"
  let array_map = "\\array_map"

  let type_structure = "\\type_structure"
end

module Typehints = struct

  let void     = "void"
  let resource = "resource"
  let num      = "num"
  let arraykey = "arraykey"
  let noreturn = "noreturn"
  let mixed    = "mixed"
  let this     = "this"

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

  let is_reserved_global_name x =
    let x = String.lowercase_ascii x in
    x = array    || x = callable || x = Classes.cSelf || x = Classes.cParent

  let is_reserved_hh_name x =
    let x = String.lowercase_ascii x in
    x = void     || x = noreturn || x = int      || x = bool     || x = float ||
    x = num      || x = string   || x = resource || x = mixed    || x = array ||
    x = arraykey || x = integer  || x = boolean  || x = double   || x = real

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

  let all_pseudo_consts = [
    g__LINE__; g__CLASS__; g__TRAIT__; g__FILE__; g__DIR__;
    g__FUNCTION__; g__METHOD__; g__NAMESPACE__
  ]
  let is_pseudo_const =
    let h = HashSet.create 23 in
    List.iter all_pseudo_consts (HashSet.add h);
    fun x -> HashSet.mem h x

end

module FB = struct

  let cEnum                  = "\\Enum"
  let cUncheckedEnum         = "\\UncheckedEnum"

  let cDynamicYield          = "\\DynamicYield"
  let cIUseDynamicYield      = "\\IUseDynamicYield"

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

end

module Shapes = struct
  let cShapes                = "\\Shapes"
  let idx                    = "idx"
  let keyExists              = "keyExists"
  let removeKey              = "removeKey"
  let toArray                = "toArray"
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
