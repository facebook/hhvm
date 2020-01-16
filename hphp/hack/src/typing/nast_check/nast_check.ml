[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

let visitor =
  Nast_visitor.iter_with
    [
      Const_prohibited_check.handler;
      Prop_modifier_prohibited_check.handler;
      Mutability_check.handler;
      Inout_check.handler;
      Naming_coroutine_check.handler;
      Interface_check.handler;
      Nast_reactivity_check.handler;
      Illegal_name_check.handler;
      Class_tparams_check.handler;
      Control_context_check.handler;
      Pocket_universes_check.handler;
      Read_from_append_check.handler;
      Dynamically_callable_attr_check.handler;
      Nast_switch_check.handler;
      Nast_generics_name_check.handler;
      Nast_class_method_check.handler;
      Global_const_check.handler;
      Duplicate_class_member_check.handler;
      Shape_name_check.handler;
      Fun_pointer_name_check.handler;
      Record_field_check.handler;
      Php_lambda_check.handler;
      Duplicate_xhp_attribute_check.handler;
    ]

let program = visitor#go

let def = visitor#go_def
