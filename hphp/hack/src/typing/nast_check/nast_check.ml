[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

let visitor =
  Nast_visitor.iter_with
    [
      Const_prohibited_check.handler;
      Prop_modifier_prohibited_check.handler;
      Inout_check.handler;
      Naming_coroutine_check.handler;
      Interface_check.handler;
      Illegal_name_check.handler;
      Class_tparams_check.handler;
      Control_context_check.handler;
      Read_from_append_check.handler;
      Dynamically_callable_attr_check.handler;
      Nast_switch_check.handler;
      Nast_generics_check.handler;
      Nast_class_method_check.handler;
      Global_const_check.handler;
      Duplicate_class_member_check.handler;
      Shape_name_check.handler;
      Fun_pointer_name_check.handler;
      Record_field_check.handler;
      Php_lambda_check.handler;
      Duplicate_xhp_attribute_check.handler;
      Attribute_nast_checks.handler;
      Trait_reuse_check.handler;
      Enum_classes_check.handler;
      Enum_supertyping_check.handler;
      List_rvalue_check.handler;
    ]

let stateful_visitor ctx =
  Stateful_aast_visitor.checker
    (Stateful_aast_visitor.combine_visitors
       (Unbound_name_check.handler ctx)
       Function_pointer_check.handler)

let program ctx p =
  let () = visitor#go ctx p in
  let v = stateful_visitor ctx in
  v#on_program v#initial_state p

let def ctx d =
  let () = visitor#go_def ctx d in
  let v = stateful_visitor ctx in
  v#on_def v#initial_state d
