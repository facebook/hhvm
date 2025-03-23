(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type t = {
  (* These options are set in both hhvm and hh config *)
  disable_lval_as_an_expression: bool;
      (** Flag to disable using lvals as expressions. *)
  const_static_props: bool;  (** Enable const static properties *)
  const_default_func_args: bool;
      (** Statically check default function arguments *)
  abstract_static_props: bool;  (** Static properties can be abstract *)
  disallow_func_ptrs_in_constants: bool;
      (** Flag to disallow HH\fun and HH\class_meth in constants and constant initializers *)
  enable_xhp_class_modifier: bool;
      (** Enable the new style xhp class.
         Old style: class :name {}
         New style: xhp class name {} *)
  disable_xhp_element_mangling: bool;
      (** Flag to disable the old stype xhp element mangling. `<something/>` would otherwise be resolved as `xhp_something`
         The new style `xhp class something {}` does not do this style of mangling, thus we need a way to disable it on the
         'lookup side'. *)
  allow_unstable_features: bool;
      (** Allows enabling unstable features via the __EnableUnstableFeatures attribute *)
  (* These options are set in hh config, but use the defaults in (from parser_options_impl.rs) hhvm *)
  hhvm_compat_mode: bool;
  hhi_mode: bool;
  codegen: bool;  (** Are we emitting bytecode? *)
  disable_legacy_soft_typehints: bool;
      (** Disable legacy soft typehint syntax (@int) and only allow the __Soft attribute. *)
  disable_legacy_attribute_syntax: bool;
      (** Disable <<...>> attribute syntax *)
  disable_xhp_children_declarations: bool;
      (** Disable `children (foo|bar+|pcdata)` declarations as they can be implemented without special syntax *)
  const_default_lambda_args: bool;
      (** Statically check default lambda arguments. Subset of default_func_args *)
  interpret_soft_types_as_like_types: bool;  (** <<__Soft>> T -> ~T *)
  is_systemlib: bool;  (** Enable features used to typecheck systemlib *)
  disallow_static_constants_in_default_func_args: bool;
  auto_namespace_map: (string * string) list;  (** Namespace aliasing map *)
  everything_sdt: bool;
      (** All classes are implcitly marked <<__SupportDynamicType>> *)
  keep_user_attributes: bool;
      (** Parse all user attributes rather than only the ones needed for typing *)
  stack_size: int;
      (** Stack size to use for parallel workers inside the parser. *)
  deregister_php_stdlib: bool;
      (** Flag for disabling functions in HHI files with the __PHPStdLib attribute *)
  union_intersection_type_hints: bool;
      (** Enables union and intersection type hints *)
  unwrap_concurrent: bool;
      (** Replace concurrent blocks with their bodies in the AST *)
  disallow_silence: bool;  (** Flag to disable the error suppression operator *)
  no_parser_readonly_check: bool;  (** Disable parser-based readonly checking *)
  disable_hh_ignore_error: int;
      (** Disable HH_IGNORE_ERROR comments, either raising an error if 1 or treating them as normal comments if 2. *)
  allowed_decl_fixme_codes: ISet.t;
      (** Set of error codes disallowed in decl positions *)
  use_legacy_experimental_feature_config: bool;
      (** Ignore the experimental_features and consider_unspecified_experimental_features_released config
          options and use a hard coded function instead *)
  experimental_features: Experimental_features.feature_status SMap.t;
      (** A mapping of names of experimental features to their status: Unstable/Preview/OngoingRelease *)
  consider_unspecified_experimental_features_released: bool;
      (** Any experimental features not specified in the experimental_features configuration field should
          default to OngoingRelease if this is true. Otherwise they will default to Unstable. This should be true for
          testing and tools that don't read .hhconfig (e.g., like hh_single_type_check and hh_parse). It should
          be false for hh_server. *)
  package_v2: bool;  (** Whether PackageV2 is enabled. *)
  package_info: PackageInfo.t;
      (** Information used to determine which package a file belongs to during typechecking. *)
  package_v2_support_multifile_tests: bool;
      (** Option for the package v2 to strip the multifile filename mangling used in Hack tests.
          Should be set to true only by the unit tests in the Hack test suite *)
  enable_class_pointer_hint: bool;
      (** When false, type hint class<T> (Hclass_ptr) becomes decl ty classname<T> (Tnewtype).
          When true, it becomes decl ty class<T> (Tclass_ptr). This option is similar to the
          interpret_soft_types_as_like_types switch. **)
  disallow_non_annotated_memoize: bool;
      (** When true, plain <<__Memoize>> will not be allowed. **)
  treat_non_annotated_memoize_as_kbic: bool;
      (** When true, plain <<__Memoize>> will be treated as <<__Memoize(#KeyedByIC)>>. **)
  use_oxidized_by_ref_decls: bool;
      (** Controls whether the direct decl parser uses oxidized-by-ref or just plain oxidized *)
}
[@@deriving show, eq]

val default : t

(* Changes here need to be synchronized with rust_parser_errors_ffi.rs *)
type ffi_t =
  bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * bool
  * Experimental_features.feature_status SMap.t
  * bool
  * bool
  * bool

val to_rust_ffi_t : t -> ffi_t
