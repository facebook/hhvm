[@@@warning "-33"]

open Hh_prelude

[@@@warning "+33"]

let visitor ctx =
  let handlers =
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
      Nast_generics_check.handler;
      Nast_class_method_check.handler;
      Global_const_check.handler;
      Duplicate_class_member_check.handler;
      Shape_name_check.handler;
      Php_lambda_check.handler;
      Duplicate_xhp_attribute_check.handler;
      Attribute_nast_checks.handler;
      List_rvalue_check.handler;
      Private_final_check.handler;
      Well_formed_internal_trait.handler;
      Type_structure_leak_check.handler;
    ]
  in
  let handlers =
    if
      TypecheckerOptions.record_fine_grained_dependencies
        (Provider_context.get_tcopt ctx)
    then
      Pessimisation_node_recording.handler :: handlers
    else
      handlers
  in

  let handlers =
    if TypecheckerOptions.skip_tast_checks (Provider_context.get_tcopt ctx) then
      []
    else
      handlers
  in
  Nast_visitor.iter_with handlers

let stateful_visitor ctx =
  Stateful_aast_visitor.checker
    (Stateful_aast_visitor.combine_visitors
       (Unbound_name_check.handler ctx)
       Function_pointer_check.handler)

let program ctx p =
  let () = (visitor ctx)#go ctx p in
  let v = stateful_visitor ctx in
  v#on_program v#initial_state p

let def ctx d =
  let () = (visitor ctx)#go_def ctx d in
  let v = stateful_visitor ctx in
  v#on_def v#initial_state d
