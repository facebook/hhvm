(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open OUnit2
open Typing_defs_constraints
module Eval = Custom_error_eval
module Ty = Typing_defs_core

let mk_ty ty_ = Ty.mk (Typing_reason.none, ty_)

(* Pattern match over a `Violated_constraint` error matching exactly the tparam
   name for which the constraint is violated *)
let test_patt_string_exactly _ =
  let open Patt_typing_error in
  let open Patt_locl_ty in
  let name = "Tviolated" in
  let pod_none = Pos_or_decl.none in
  let snd_err =
    Typing_error.Secondary.Violated_constraint
      {
        is_coeffect = false;
        cstrs = [(pod_none, (pod_none, name))];
        ty_sub = LoclType (Ty.mk (Typing_reason.none, Ty.Tdynamic));
        ty_sup = LoclType (Ty.mk (Typing_reason.none, Ty.Tdynamic));
      }
  in
  let err =
    Typing_error.(
      apply_reasons ~on_error:(Reasons_callback.unify_error_at Pos.none) snd_err)
  in
  (* Matches [Apply_reasons] error with any callback, applied to
     [Violated_constraint] secondary error with contrained tparam `Tviolated` *)
  let patt =
    Custom_error.Error_v1
      (Apply_reasons
         {
           patt_rsns_cb = Any_reasons_callback;
           patt_secondary =
             Violated_constraint
               {
                 patt_cstr = Patt_string.Exactly name;
                 patt_ty_sub = Any;
                 patt_ty_sup = Any;
               };
         })
  in
  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Ok"] }
  in
  let custom_err = Custom_error.{ name; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in
  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Ok"]]

(* Pattern match over the [tysub] contained in a `Violated_constraint` error;
   the type match requires an exact match on the class name and for it to
   have exactly one type param. The type param must be a shape containing
   a field name "a" which can be of any type *)
let test_patt_tysub _ =
  let open Patt_typing_error in
  let open Patt_locl_ty in
  let param_name = "Tviolated" in
  let class_name = "Classy" in
  let pod_none = Pos_or_decl.none in
  let shp =
    Ty.(
      Tshape
        {
          s_origin = Missing_origin;
          s_unknown_value = mk_ty Tdynamic;
          s_fields =
            TShapeMap.of_list
              [
                ( TSFlit_str (pod_none, "c"),
                  { sft_optional = false; sft_ty = mk_ty Tdynamic } );
                ( TSFlit_str (pod_none, "a"),
                  { sft_optional = false; sft_ty = mk_ty Tdynamic } );
                ( TSFlit_str (pod_none, "b"),
                  { sft_optional = false; sft_ty = mk_ty Tdynamic } );
              ];
        })
  in
  let ty_locl_sub =
    Ty.Tclass ((pod_none, "\\" ^ class_name), Ty.nonexact, [mk_ty shp])
  in
  let snd_err =
    Typing_error.Secondary.Violated_constraint
      {
        is_coeffect = false;
        cstrs = [(pod_none, (pod_none, param_name))];
        ty_sub = LoclType (Ty.mk (Typing_reason.none, ty_locl_sub));
        ty_sup = LoclType (Ty.mk (Typing_reason.none, Ty.Tdynamic));
      }
  in
  let err =
    Typing_error.(
      apply_reasons ~on_error:(Reasons_callback.unify_error_at Pos.none) snd_err)
  in

  (* Matches a class with exactly one parameter where that param has a shape
     type with one field named 'a' which can have any type *)
  let patt_ty_sub =
    Patt_locl_ty.Apply
      {
        patt_name =
          Patt_name.Name
            {
              patt_namespace = Patt_name.Root;
              patt_name = Patt_string.Exactly class_name;
            };
        patt_params =
          Cons
            {
              patt_hd =
                Shape
                  (Fld
                     {
                       patt_fld =
                         {
                           lbl = StrLbl "a";
                           optional = false;
                           patt = As { lbl = "x"; patt = Any };
                         };
                       patt_rest = Open;
                     });
              patt_tl = Nil;
            };
      }
  in
  (* Match the subtype in our error message *)
  let patt =
    Custom_error.Error_v1
      (Apply_reasons
         {
           patt_rsns_cb = Any_reasons_callback;
           patt_secondary =
             Violated_constraint
               {
                 patt_cstr = Patt_string.Exactly param_name;
                 patt_ty_sub;
                 patt_ty_sup = Any;
               };
         })
  in
  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Ok:"; Ty_var "x"] }
  in
  let custom_err = Custom_error.{ name = "patt tysub"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in
  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [Either.[First "Ok:"; Second (Eval.Value.Ty (mk_ty Ty.Tdynamic))]]

(* Pattern match over the [tysub] contained in a `Violated_constraint` error;
   the type match requires an exact match on the class name and for it to
   have exactly one type param. The type parameter may _either_ be an
   arraykey (which we bind to `x`) or a shape with a field named `a` (whose
   type we bind to `x`) *)
let test_patt_tysub_or_pattern _ =
  let open Patt_typing_error in
  let open Patt_locl_ty in
  let param_name = "Tviolated" in
  let class_name = "Classy" in
  let pod_none = Pos_or_decl.none in
  let shp =
    Ty.(
      Tshape
        {
          s_origin = Missing_origin;
          s_unknown_value = mk_ty Tdynamic;
          s_fields =
            TShapeMap.of_list
              [
                ( TSFlit_str (pod_none, "c"),
                  { sft_optional = false; sft_ty = mk_ty Tdynamic } );
                ( TSFlit_str (pod_none, "a"),
                  { sft_optional = false; sft_ty = mk_ty Tdynamic } );
                ( TSFlit_str (pod_none, "b"),
                  { sft_optional = false; sft_ty = mk_ty Tdynamic } );
              ];
        })
  in
  let ty_locl_sub =
    Ty.Tclass ((pod_none, "\\" ^ class_name), Ty.nonexact, [mk_ty shp])
  in
  let snd_err =
    Typing_error.Secondary.Violated_constraint
      {
        is_coeffect = false;
        cstrs = [(pod_none, (pod_none, param_name))];
        ty_sub = LoclType (Ty.mk (Typing_reason.none, ty_locl_sub));
        ty_sup = LoclType (Ty.mk (Typing_reason.none, Ty.Tdynamic));
      }
  in
  let err =
    Typing_error.(
      apply_reasons ~on_error:(Reasons_callback.unify_error_at Pos.none) snd_err)
  in

  (* Matches a class with exactly one parameter where that param has a shape
     type with one field named 'a' which can have any type *)
  let patt_ty_sub =
    Patt_locl_ty.Apply
      {
        patt_name =
          Patt_name.Name
            {
              patt_namespace = Patt_name.Root;
              patt_name = Patt_string.Exactly class_name;
            };
        patt_params =
          Cons
            {
              patt_hd =
                Or
                  {
                    patt_fst = As { lbl = "x"; patt = Prim Arraykey };
                    patt_snd =
                      Shape
                        (Fld
                           {
                             patt_fld =
                               {
                                 lbl = StrLbl "a";
                                 optional = false;
                                 patt = As { lbl = "x"; patt = Any };
                               };
                             patt_rest = Open;
                           });
                  };
              patt_tl = Nil;
            };
      }
  in
  (* Match the subtype in our error message *)
  let patt =
    Custom_error.Error_v1
      (Apply_reasons
         {
           patt_rsns_cb = Any_reasons_callback;
           patt_secondary =
             Violated_constraint
               {
                 patt_cstr = Patt_string.Exactly param_name;
                 patt_ty_sub;
                 patt_ty_sup = Any;
               };
         })
  in
  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Ok:"; Ty_var "x"] }
  in
  let custom_err = Custom_error.{ name = "patt tysub"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in
  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [Either.[First "Ok:"; Second (Eval.Value.Ty (mk_ty Ty.Tdynamic))]]

let test_namespace _ =
  let pod_none = Pos_or_decl.none in
  let param_name = "T" in
  let class_name = "Classy" in
  let ns = ["This"; "Is"; "The"; "Namespace"] in
  let rendered_name = String.concat ~sep:"\\" (ns @ [class_name]) in
  let ty = Ty.Tclass ((pod_none, rendered_name), Ty.nonexact, []) in
  let patt_namespace =
    Core.List.fold_left
      ~f:(fun prefix str ->
        Patt_name.Slash { prefix; elt = Patt_string.Exactly str })
      ~init:Patt_name.Root
      ns
  in
  let patt_ty_sub =
    Patt_locl_ty.(
      Apply
        {
          patt_name =
            Patt_name.Name
              { patt_namespace; patt_name = Patt_string.Exactly class_name };
          patt_params = Nil;
        })
  in
  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Apply_reasons
          {
            patt_rsns_cb = Any_reasons_callback;
            patt_secondary =
              Violated_constraint
                {
                  patt_cstr = Patt_string.Exactly param_name;
                  patt_ty_sub;
                  patt_ty_sup = Patt_locl_ty.Any;
                };
          })
  in
  let snd_err =
    Typing_error.Secondary.Violated_constraint
      {
        is_coeffect = false;
        cstrs = [(pod_none, (pod_none, param_name))];
        ty_sub = LoclType (Ty.mk (Typing_reason.none, ty));
        ty_sup = LoclType (Ty.mk (Typing_reason.none, Ty.Tdynamic));
      }
  in
  let err =
    Typing_error.(
      apply_reasons ~on_error:(Reasons_callback.unify_error_at Pos.none) snd_err)
  in
  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Ok"] }
  in
  let custom_err = Custom_error.{ name = "namespaced"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in
  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [Either.[First "Ok"]]

(* ============================================================================ *)
(* Member_not_found Pattern Tests *)
(* ============================================================================ *)

(* Test instance method pattern matching *)
let test_member_not_found_instance_method_pattern _ =
  let class_name = "MyClass" in
  let method_name = "nonexistentMethod" in

  let prim_err =
    Typing_error.Primary.Member_not_found
      {
        pos = Pos.none;
        kind = `method_;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = method_name;
        hint = lazy None;
        reason = lazy [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Instance_only;
               patt_kind = Method_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name { patt_string = Patt_string.Exactly method_name });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Instance method not found in class"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Instance method not found in class"]]

(* Test instance property pattern matching *)
let test_member_not_found_instance_property_pattern _ =
  let class_name = "MyClass" in
  let property_name = "nonexistentProperty" in

  let prim_err =
    Typing_error.Primary.Member_not_found
      {
        pos = Pos.none;
        kind = `property;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = property_name;
        hint = lazy None;
        reason = lazy [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Instance_only;
               patt_kind = Property_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name
                     { patt_string = Patt_string.Exactly property_name });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Instance property not found in class"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Instance property not found in class"]]

(* Test static method pattern matching *)
let test_member_not_found_static_method_pattern _ =
  let class_name = "MyClass" in
  let method_name = "nonexistentStaticMethod" in

  let prim_err =
    Typing_error.Primary.Smember_not_found
      {
        pos = Pos.none;
        kind = `static_method;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = method_name;
        hint = None;
        quickfixes = [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Static_only;
               patt_kind = Method_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name { patt_string = Patt_string.Exactly method_name });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Static method not found in class"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Static method not found in class"]]

(* Test class constant pattern matching *)
let test_member_not_found_class_constant_pattern _ =
  let class_name = "MyClass" in
  let constant_name = "NONEXISTENT_CONSTANT" in

  let prim_err =
    Typing_error.Primary.Smember_not_found
      {
        pos = Pos.none;
        kind = `class_constant;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = constant_name;
        hint = None;
        quickfixes = [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Static_only;
               patt_kind = Class_constant_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name
                     { patt_string = Patt_string.Exactly constant_name });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Class constant not found"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Class constant not found"]]

(* Test class variable (static property) pattern matching *)
let test_member_not_found_static_property_pattern _ =
  let class_name = "MyClass" in
  let property_name = "nonexistentStaticProperty" in

  let prim_err =
    Typing_error.Primary.Smember_not_found
      {
        pos = Pos.none;
        kind = `class_variable;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = property_name;
        hint = None;
        quickfixes = [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Static_only;
               patt_kind = Property_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name
                     { patt_string = Patt_string.Exactly property_name });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Static property not found"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Static property not found"]]

(* Test class typeconst pattern matching *)
let test_member_not_found_class_typeconst_pattern _ =
  let class_name = "MyClass" in
  let typeconst_name = "TMyType" in

  let prim_err =
    Typing_error.Primary.Smember_not_found
      {
        pos = Pos.none;
        kind = `class_typeconst;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = typeconst_name;
        hint = None;
        quickfixes = [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Static_only;
               patt_kind = Class_typeconst_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name
                     { patt_string = Patt_string.Exactly typeconst_name });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Type constant not found"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Type constant not found"]]

(* Test wildcard pattern matching - Any_member_kind *)
let test_member_wildcard_patterns _ =
  let class_name = "MyClass" in
  let method_name = "anyMethod" in

  let prim_err =
    Typing_error.Primary.Member_not_found
      {
        pos = Pos.none;
        kind = `method_;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = method_name;
        hint = lazy None;
        reason = lazy [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = None;
               patt_kind = Any_member_kind;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name { patt_string = Patt_string.Exactly method_name });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Any member not found"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Any member not found"]]

(* Test string pattern matching with regex-like patterns *)
let test_member_name_patterns _ =
  let class_name = "TestClass" in
  let method_name = "renderAsync" in

  let prim_err =
    Typing_error.Primary.Member_not_found
      {
        pos = Pos.none;
        kind = `method_;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = method_name;
        hint = lazy None;
        reason = lazy [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Instance_only;
               patt_kind = Method_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Contains "Test";
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name
                     { patt_string = Patt_string.Starts_with "render" });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Render method not found in test class"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Render method not found in test class"]]

(* Test pattern specificity - specific patterns should match before general ones *)
let test_member_pattern_specificity _ =
  let class_name = "MyClass" in
  let method_name = "testMethod" in

  let prim_err =
    Typing_error.Primary.Member_not_found
      {
        pos = Pos.none;
        kind = `method_;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = method_name;
        hint = lazy None;
        reason = lazy [];
      }
  in

  let err = Typing_error.primary prim_err in

  (* Specific pattern should match first *)
  let specific_patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Instance_only;
               patt_kind = Method_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name { patt_string = Patt_string.Exactly method_name });
               patt_visibility = None;
             }))
  in

  (* General pattern *)
  let general_patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = None;
               patt_kind = Any_member_kind;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name { patt_string = Patt_string.Exactly method_name });
               patt_visibility = None;
             }))
  in

  let specific_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "Specific method error"] }
  in
  let general_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "General method error"] }
  in

  let specific_err =
    Custom_error.
      {
        name = "specific";
        patt = specific_patt;
        error_message = specific_message;
      }
  in
  let general_err =
    Custom_error.
      { name = "general"; patt = general_patt; error_message = general_message }
  in

  let custom_config =
    Custom_error_config.{ valid = [specific_err; general_err]; invalid = [] }
  in

  let open Core in
  (* First (more specific) pattern should match *)
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [
      [Either.First "Specific method error"];
      [Either.First "General method error"];
    ]

(* Test no match when pattern doesn't fit *)
let test_member_pattern_no_match _ =
  let class_name = "MyClass" in
  let method_name = "testMethod" in

  let prim_err =
    Typing_error.Primary.Member_not_found
      {
        pos = Pos.none;
        kind = `method_;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = method_name;
        hint = lazy None;
        reason = lazy [];
      }
  in

  let err = Typing_error.primary prim_err in

  (* Pattern that shouldn't match - expecting static but got instance *)
  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Static_only;
               patt_kind = Method_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly class_name;
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name { patt_string = Patt_string.Exactly method_name });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Should not match"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  (* Should return empty list since pattern doesn't match *)
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    []

(* Test complex string patterns with OR logic *)
let test_member_complex_string_patterns _ =
  let class_name = "ReactComponent" in
  let method_name = "renderToString" in

  let prim_err =
    Typing_error.Primary.Member_not_found
      {
        pos = Pos.none;
        kind = `method_;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = method_name;
        hint = lazy None;
        reason = lazy [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Instance_only;
               patt_kind = Method_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name =
                       Patt_string.Or
                         [
                           Patt_string.Contains "React";
                           Patt_string.Contains "Component";
                         ];
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name
                     {
                       patt_string =
                         Patt_string.And
                           [
                             Patt_string.Starts_with "render";
                             Patt_string.Contains "String";
                           ];
                     });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "React render method not found"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "React render method not found"]]

(* Test handling of malformed class names *)
let test_malformed_class_name _ =
  let class_name = "\\\\\\" in
  let method_name = "whatever" in

  let prim_err =
    Typing_error.Primary.Member_not_found
      {
        pos = Pos.none;
        kind = `method_;
        class_name;
        class_pos = Pos_or_decl.none;
        member_name = method_name;
        hint = lazy None;
        reason = lazy [];
      }
  in

  let err = Typing_error.primary prim_err in

  let patt =
    Custom_error.Error_v1
      Patt_typing_error.(
        Primary
          (Member_not_found
             {
               patt_is_static = Some Instance_only;
               patt_kind = Method_only;
               patt_class_name =
                 Patt_name.Name
                   {
                     patt_namespace = Patt_name.Root;
                     patt_name = Patt_string.Exactly "RealClassName";
                   };
               patt_member_name =
                 Patt_member_name.(
                   Member_name { patt_string = Patt_string.Exactly method_name });
               patt_visibility = None;
             }))
  in

  let error_message =
    Custom_error.Message_v1
      Error_message.{ message = [Lit "React render method not found"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    (* NoMatch *)
    []
(* == Package errors ======================================================== *)

let test_cross_pkg_access _ =
  let open Typing_error in
  let pos_file = Relative_path.from_root ~suffix:"foo/bar/baz.php" in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access
         {
           pos;
           decl_pos = Pos_or_decl.none;
           (* Everything else is currently not matched on in the custom error *)
           current_package_pos = Pos.none;
           current_package_def_pos = Pos.none;
           current_package_name = None;
           current_package_assignment_kind = "whatever";
           target_package_pos = Pos.none;
           target_package_name = None;
           target_package_assignment_kind = "whatever";
           current_filename = Relative_path.default;
           target_filename = Relative_path.default;
           target_id = "whatever";
           target_symbol_spec = "whatever";
         })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path =
    Some Patt_file.(dot </> Patt_string.Exactly "foo" </> Patt_string.Wildcard)
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      {
        patt_file_path;
        patt_file_name;
        patt_file_extension;
        allow_glob = false;
      }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Boom"]]

let test_cross_pkg_access_with_requirepackage _ =
  let open Typing_error in
  let pos_file = Relative_path.from_root ~suffix:"foo/bar/baz.php" in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access_with_requirepackage
         { pos; decl_pos = Pos_or_decl.none; target_package = "test_package" })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path =
    Some Patt_file.(dot </> Patt_string.Exactly "foo" </> Patt_string.Wildcard)
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      {
        patt_file_path;
        patt_file_name;
        patt_file_extension;
        allow_glob = false;
      }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access_with_requirepackage
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Boom"]]

let test_optional_path_segment_missing _ =
  let open Typing_error in
  let pos_file = Relative_path.from_root ~suffix:"foo/bar/baz.php" in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access
         {
           pos;
           decl_pos = Pos_or_decl.none;
           (* Everything else is currently not matched on in the custom error *)
           current_package_pos = Pos.none;
           current_package_def_pos = Pos.none;
           current_package_name = None;
           current_package_assignment_kind = "whatever";
           target_package_pos = Pos.none;
           target_package_name = None;
           target_package_assignment_kind = "whatever";
           current_filename = Relative_path.default;
           target_filename = Relative_path.default;
           target_id = "whatever";
           target_symbol_spec = "whatever";
         })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path =
    Some
      Patt_file.(
        dot
        </> Patt_string.Exactly "foo"
        </?> Patt_string.Exactly "qux"
        </> Patt_string.Wildcard)
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      {
        patt_file_path;
        patt_file_name;
        patt_file_extension;
        allow_glob = false;
      }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Boom"]]

let test_optional_path_segment_present _ =
  let open Typing_error in
  let pos_file = Relative_path.from_root ~suffix:"foo/qux/bar/baz.php" in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access
         {
           pos;
           decl_pos = Pos_or_decl.none;
           (* Everything else is currently not matched on in the custom error *)
           current_package_pos = Pos.none;
           current_package_def_pos = Pos.none;
           current_package_name = None;
           current_package_assignment_kind = "whatever";
           target_package_pos = Pos.none;
           target_package_name = None;
           target_package_assignment_kind = "whatever";
           current_filename = Relative_path.default;
           target_filename = Relative_path.default;
           target_id = "whatever";
           target_symbol_spec = "whatever";
         })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path =
    Some
      Patt_file.(
        dot
        </> Patt_string.Exactly "foo"
        </?> Patt_string.Exactly "qux"
        </> Patt_string.Wildcard)
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      {
        patt_file_path;
        patt_file_name;
        patt_file_extension;
        allow_glob = false;
      }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Boom"]]

let test_path_glob _ =
  let open Typing_error in
  let pos_file = Relative_path.from_root ~suffix:"foo/bar/qux/fizz/baz.php" in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access
         {
           pos;
           decl_pos = Pos_or_decl.none;
           (* Everything else is currently not matched on in the custom error *)
           current_package_pos = Pos.none;
           current_package_def_pos = Pos.none;
           current_package_name = None;
           current_package_assignment_kind = "whatever";
           target_package_pos = Pos.none;
           target_package_name = None;
           target_package_assignment_kind = "whatever";
           current_filename = Relative_path.default;
           target_filename = Relative_path.default;
           target_id = "whatever";
           target_symbol_spec = "whatever";
         })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path = Some Patt_file.(dot </> Patt_string.Exactly "foo")
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      { patt_file_path; patt_file_name; patt_file_extension; allow_glob = true }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Boom"]]

let test_path_glob_with_optional_present _ =
  let open Typing_error in
  let pos_file =
    Relative_path.from_root ~suffix:"foo/wibble/bar/qux/fizz/baz.php"
  in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access
         {
           pos;
           decl_pos = Pos_or_decl.none;
           (* Everything else is currently not matched on in the custom error *)
           current_package_pos = Pos.none;
           current_package_def_pos = Pos.none;
           current_package_name = None;
           current_package_assignment_kind = "whatever";
           target_package_pos = Pos.none;
           target_package_name = None;
           target_package_assignment_kind = "whatever";
           current_filename = Relative_path.default;
           target_filename = Relative_path.default;
           target_id = "whatever";
           target_symbol_spec = "whatever";
         })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path =
    Some
      Patt_file.(
        dot </> Patt_string.Exactly "foo" </?> Patt_string.Exactly "wibble")
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      { patt_file_path; patt_file_name; patt_file_extension; allow_glob = true }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Boom"]]

let test_path_glob_with_optional_missing _ =
  let open Typing_error in
  let pos_file = Relative_path.from_root ~suffix:"foo/bar/qux/fizz/baz.php" in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access
         {
           pos;
           decl_pos = Pos_or_decl.none;
           (* Everything else is currently not matched on in the custom error *)
           current_package_pos = Pos.none;
           current_package_def_pos = Pos.none;
           current_package_name = None;
           current_package_assignment_kind = "whatever";
           target_package_pos = Pos.none;
           target_package_name = None;
           target_package_assignment_kind = "whatever";
           current_filename = Relative_path.default;
           target_filename = Relative_path.default;
           target_id = "whatever";
           target_symbol_spec = "whatever";
         })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path =
    Some
      Patt_file.(
        dot </> Patt_string.Exactly "foo" </?> Patt_string.Exactly "wibble")
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      { patt_file_path; patt_file_name; patt_file_extension; allow_glob = true }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Boom"]]

let test_path_glob_bad _ =
  let open Typing_error in
  let pos_file = Relative_path.from_root ~suffix:"foo/baz.php" in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access
         {
           pos;
           decl_pos = Pos_or_decl.none;
           (* Everything else is currently not matched on in the custom error *)
           current_package_pos = Pos.none;
           current_package_def_pos = Pos.none;
           current_package_name = None;
           current_package_assignment_kind = "whatever";
           target_package_pos = Pos.none;
           target_package_name = None;
           target_package_assignment_kind = "whatever";
           current_filename = Relative_path.default;
           target_filename = Relative_path.default;
           target_id = "whatever";
           target_symbol_spec = "whatever";
         })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path =
    Some
      Patt_file.(
        dot
        </> Patt_string.Exactly "foo"
        </> Patt_string.Exactly "__generated__")
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      { patt_file_path; patt_file_name; patt_file_extension; allow_glob = true }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    (* NoMatch *)
    []

let test_path_glob_with_optional_missing_bad _ =
  let open Typing_error in
  let pos_file = Relative_path.from_root ~suffix:"foo/baz.php" in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access
         {
           pos;
           decl_pos = Pos_or_decl.none;
           (* Everything else is currently not matched on in the custom error *)
           current_package_pos = Pos.none;
           current_package_def_pos = Pos.none;
           current_package_name = None;
           current_package_assignment_kind = "whatever";
           target_package_pos = Pos.none;
           target_package_name = None;
           target_package_assignment_kind = "whatever";
           current_filename = Relative_path.default;
           target_filename = Relative_path.default;
           target_id = "whatever";
           target_symbol_spec = "whatever";
         })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path =
    Some
      Patt_file.(
        dot
        </> Patt_string.Exactly "foo"
        </?> Patt_string.Exactly "bar"
        </> Patt_string.Exactly "__generated__")
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      { patt_file_path; patt_file_name; patt_file_extension; allow_glob = true }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    (* NoMatch *)
    []

let test_path_glob_with_optional_present_bad _ =
  let open Typing_error in
  let pos_file = Relative_path.from_root ~suffix:"foo/bar/baz.php" in
  let pos =
    Pos.make_from_lnum_bol_offset
      ~pos_file
      ~pos_start:(0, 0, 0)
      ~pos_end:(0, 0, 0)
  in
  let prim_err =
    Primary.Package
      (Primary.Package.Cross_pkg_access
         {
           pos;
           decl_pos = Pos_or_decl.none;
           (* Everything else is currently not matched on in the custom error *)
           current_package_pos = Pos.none;
           current_package_def_pos = Pos.none;
           current_package_name = None;
           current_package_assignment_kind = "whatever";
           target_package_pos = Pos.none;
           target_package_name = None;
           target_package_assignment_kind = "whatever";
           current_filename = Relative_path.default;
           target_filename = Relative_path.default;
           target_id = "whatever";
           target_symbol_spec = "whatever";
         })
  in
  let err = Typing_error.primary prim_err in
  (* match 'foo/*' *)
  let patt_file_path =
    Some
      Patt_file.(
        dot
        </> Patt_string.Exactly "foo"
        </?> Patt_string.Exactly "bar"
        </> Patt_string.Exactly "__generated__")
  and patt_file_name = Patt_string.Starts_with "b"
  and patt_file_extension = Patt_string.Exactly "php" in
  let patt_use_file =
    Patt_file.Name
      { patt_file_path; patt_file_name; patt_file_extension; allow_glob = true }
  in
  let patt_primary =
    Patt_typing_error.Primary
      (Patt_typing_error.Cross_pkg_access
         { patt_use_file; patt_decl_file = Patt_file.Wildcard })
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_primary) in

  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in

  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    (* NoMatch *)
    []
(* ~~ Naming errors ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)

let test_unbound_name_any_context _ =
  let name = "foo" in
  let err =
    Naming_error.Unbound_name
      { pos = Pos.none; name; kind = Name_context.FunctionNamespace }
  in

  let patt_namespace = Patt_name.Root in
  let patt_name =
    Patt_name.Name { patt_namespace; patt_name = Patt_string.Exactly name }
  in
  let patt =
    Custom_error.Error_v2
      (Patt_error.Naming
         Patt_naming_error.(
           Unbound_name { patt_name; patt_name_context = Any_name_context }))
  in
  let msg_str = "foo doesn't exist" in
  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit msg_str] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in
  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_naming_error custom_config ~err)
    [[Either.First msg_str]]

let test_unbound_name_function_context_good _ =
  let name = "foo" in
  let err =
    Naming_error.Unbound_name
      { pos = Pos.none; name; kind = Name_context.FunctionNamespace }
  in

  let patt_namespace = Patt_name.Root in
  let patt_name =
    Patt_name.Name { patt_namespace; patt_name = Patt_string.Exactly name }
  in
  let patt =
    Custom_error.Error_v2
      (Patt_error.Naming
         Patt_naming_error.(
           Unbound_name
             {
               patt_name;
               patt_name_context =
                 One_of_name_context [Name_context.FunctionNamespace];
             }))
  in
  let msg_str = "foo doesn't exist" in
  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit msg_str] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in
  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_naming_error custom_config ~err)
    [[Either.First msg_str]]

let test_unbound_name_function_context_bad _ =
  let name = "foo" in
  let err =
    Naming_error.Unbound_name
      { pos = Pos.none; name; kind = Name_context.FunctionNamespace }
  in

  let patt_namespace = Patt_name.Root in
  let patt_name =
    Patt_name.Name { patt_namespace; patt_name = Patt_string.Exactly name }
  in
  let patt =
    Custom_error.Error_v2
      (Patt_error.Naming
         Patt_naming_error.(
           Unbound_name
             {
               patt_name;
               patt_name_context =
                 One_of_name_context [Name_context.TypeNamespace];
             }))
  in
  let msg_str = "foo doesn't exist" in
  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit msg_str] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in
  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_naming_error custom_config ~err)
    (* NoMatch *)
    []

(* -- Expression trees ------------------------------------------------------ *)

let test_expr_tree_unsupported_operator _ =
  let class_name = "//BKSClientVoid" and member_name = "__unwrap" in
  let expr_tree_err =
    Typing_error.Primary.(
      Expr_tree.Expression_tree_unsupported_operator
        { pos = Pos.none; member_name; class_name })
  in
  let primary_error = Typing_error.Primary.Expr_tree expr_tree_err in
  let err = Typing_error.primary primary_error in

  let patt_namespace = Patt_name.Root in
  let patt_class_name =
    Patt_name.Name
      { patt_namespace; patt_name = Patt_string.Exactly class_name }
  in
  let patt_member_name =
    Patt_member_name.Member_name
      { patt_string = Patt_string.Exactly member_name }
  in
  let patt_typing_error =
    Patt_typing_error.(
      Primary
        (Expression_tree_unsupported_operator
           { patt_class_name; patt_member_name }))
  in
  let patt = Custom_error.Error_v2 (Patt_error.Typing patt_typing_error) in
  let error_message =
    Custom_error.Message_v1 Error_message.{ message = [Lit "Boom"] }
  in
  let custom_err = Custom_error.{ name = "test"; patt; error_message } in
  let custom_config =
    Custom_error_config.{ valid = [custom_err]; invalid = [] }
  in
  let open Core in
  assert_equal
    ~cmp:[%compare.equal: (string, Eval.Value.t) Either.t list list]
    (Eval.eval_typing_error custom_config ~err)
    [[Either.First "Boom"]]

(* ~~ Test suite ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ *)
let tests =
  [
    "test_patt_string_exactly" >:: test_patt_string_exactly;
    "test_patt_tysub" >:: test_patt_tysub;
    "test_patt_tysub_or_pattern" >:: test_patt_tysub_or_pattern;
    "test_namespace" >:: test_namespace;
    "test_member_not_found_instance_method_pattern"
    >:: test_member_not_found_instance_method_pattern;
    "test_member_not_found_instance_property_pattern"
    >:: test_member_not_found_instance_property_pattern;
    "test_member_not_found_static_method_pattern"
    >:: test_member_not_found_static_method_pattern;
    "test_member_not_found_class_constant_pattern"
    >:: test_member_not_found_class_constant_pattern;
    "test_member_not_found_static_property_pattern"
    >:: test_member_not_found_static_property_pattern;
    "test_member_not_found_class_typeconst_pattern"
    >:: test_member_not_found_class_typeconst_pattern;
    "test_member_wildcard_patterns" >:: test_member_wildcard_patterns;
    "test_member_name_patterns" >:: test_member_name_patterns;
    "test_member_pattern_specificity" >:: test_member_pattern_specificity;
    "test_member_pattern_no_match" >:: test_member_pattern_no_match;
    "test_member_complex_string_patterns"
    >:: test_member_complex_string_patterns;
    "test_unbound_name_any_context" >:: test_unbound_name_any_context;
    "test_unbound_name_function_context_good"
    >:: test_unbound_name_function_context_good;
    "test_unbound_name_function_context_bad"
    >:: test_unbound_name_function_context_bad;
    "test_cross_pkg_access" >:: test_cross_pkg_access;
    "test_cross_pkg_access_with_requirepackage"
    >:: test_cross_pkg_access_with_requirepackage;
    "test_malformed_class_name" >:: test_malformed_class_name;
    "test_optional_path_segment_missing" >:: test_optional_path_segment_missing;
    "test_optional_path_segment_present" >:: test_optional_path_segment_present;
    "test_path_glob" >:: test_path_glob;
    "test_path_glob_with_optional_present"
    >:: test_path_glob_with_optional_present;
    "test_path_glob_with_optional_missing"
    >:: test_path_glob_with_optional_missing;
    "test_expr_tree_unsupported_operator"
    >:: test_expr_tree_unsupported_operator;
    "test_path_glob_bad" >:: test_path_glob_bad;
    "test_path_glob_with_optional_missing_bad"
    >:: test_path_glob_with_optional_missing_bad;
    "test_path_glob_with_optional_present_bad"
    >:: test_path_glob_with_optional_present_bad;
  ]

let () = "custom_error_unit_tests" >::: tests |> run_test_tt_main
