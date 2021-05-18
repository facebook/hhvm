(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(**
 * Classes that implement this will be called at every node in the AST
 *   Used to implement NAST error checking
 *   The order of the nodes traversed will be from root (program) to leaf
 *   Each method will always be called at their respective nodes
 *   Each class can maintain their own state which will be threaded to children
 *)

class type virtual ['env] nast_visitor_with_state =
  object
    method virtual initial_state : 'env

    method at_fun_def : 'env -> Nast.fun_def -> 'env

    method at_class_ : 'env -> Nast.class_ -> 'env

    method at_method_ : 'env -> Nast.method_ -> 'env

    method at_record_def : 'env -> Nast.record_def -> 'env

    method at_expr : 'env -> Nast.expr -> 'env

    method at_stmt : 'env -> Nast.stmt -> 'env

    method at_hint : 'env -> Aast.hint -> 'env

    method at_typedef : 'env -> Nast.typedef -> 'env

    method at_gconst : 'env -> Nast.gconst -> 'env

    method at_file_attribute : 'env -> Nast.file_attribute -> 'env

    method at_shape_field_name : 'env -> Nast.shape_field_name -> 'env

    method at_user_attribute : 'env -> Nast.user_attribute -> 'env

    method at_class_id : 'env -> Nast.class_id -> 'env

    method at_catch : 'env -> Nast.catch -> 'env

    method at_targ : 'env -> Nast.targ -> 'env

    method at_class_hint : 'env -> Nast.class_hint -> 'env

    method at_trait_hint : 'env -> Nast.trait_hint -> 'env

    method at_record_hint : 'env -> Nast.record_hint -> 'env

    method at_xhp_attr_hint : 'env -> Nast.xhp_attr_hint -> 'env
  end

class virtual ['env] default_nast_visitor_with_state :
  ['env] nast_visitor_with_state =
  object
    method virtual initial_state : 'env

    method at_fun_def env _ = env

    method at_class_ env _ = env

    method at_method_ env _ = env

    method at_record_def env _ = env

    method at_expr env _ = env

    method at_stmt env _ = env

    method at_hint env _ = env

    method at_typedef env _ = env

    method at_gconst env _ = env

    method at_file_attribute env _ = env

    method at_shape_field_name env _ = env

    method at_user_attribute env _ = env

    method at_class_id env _ = env

    method at_catch env _ = env

    method at_targ env _ = env

    method at_class_hint env _ = env

    method at_trait_hint env _ = env

    method at_record_hint env _ = env

    method at_xhp_attr_hint env _ = env
  end

let combine_visitors =
  let visit v1 v2 (env1, env2) node = (v1 env1 node, v2 env2 node) in
  let combine
      (visitor1 : 'env1 nast_visitor_with_state)
      (visitor2 : 'env2 nast_visitor_with_state) :
      ('env1 * 'env2) nast_visitor_with_state =
    object
      method initial_state = (visitor1#initial_state, visitor2#initial_state)

      method at_fun_def = visit visitor1#at_fun_def visitor2#at_fun_def

      method at_class_ = visit visitor1#at_class_ visitor2#at_class_

      method at_method_ = visit visitor1#at_method_ visitor2#at_method_

      method at_record_def = visit visitor1#at_record_def visitor2#at_record_def

      method at_expr = visit visitor1#at_expr visitor2#at_expr

      method at_stmt = visit visitor1#at_stmt visitor2#at_stmt

      method at_hint = visit visitor1#at_hint visitor2#at_hint

      method at_typedef = visit visitor1#at_typedef visitor2#at_typedef

      method at_gconst = visit visitor1#at_gconst visitor2#at_gconst

      method at_file_attribute =
        visit visitor1#at_file_attribute visitor2#at_file_attribute

      method at_shape_field_name =
        visit visitor1#at_shape_field_name visitor2#at_shape_field_name

      method at_user_attribute =
        visit visitor1#at_user_attribute visitor2#at_user_attribute

      method at_class_id = visit visitor1#at_class_id visitor2#at_class_id

      method at_catch = visit visitor1#at_catch visitor2#at_catch

      method at_targ = visit visitor1#at_targ visitor2#at_targ

      method at_class_hint = visit visitor1#at_class_hint visitor2#at_class_hint

      method at_trait_hint = visit visitor1#at_trait_hint visitor2#at_trait_hint

      method at_record_hint =
        visit visitor1#at_record_hint visitor2#at_record_hint

      method at_xhp_attr_hint =
        visit visitor1#at_xhp_attr_hint visitor2#at_xhp_attr_hint
    end
  in
  combine

let checker (visitor : 'env nast_visitor_with_state) =
  object
    inherit [_] Aast.iter as super

    method initial_state = visitor#initial_state

    method! on_fun_def env f =
      let env = visitor#at_fun_def env f in
      super#on_fun_def env f

    method! on_class_ env c =
      let env = visitor#at_class_ env c in
      super#on_class_ env c

    method! on_method_ env m =
      let env = visitor#at_method_ env m in
      super#on_method_ env m

    method! on_record_def env rd =
      let env = visitor#at_record_def env rd in
      super#on_record_def env rd

    method! on_expr env e =
      let env = visitor#at_expr env e in
      super#on_expr env e

    method! on_stmt env s =
      let env = visitor#at_stmt env s in
      super#on_stmt env s

    method! on_hint env h =
      let env = visitor#at_hint env h in
      super#on_hint env h

    method! on_typedef env td =
      let env = visitor#at_typedef env td in
      super#on_typedef env td

    method! on_gconst env gc =
      let env = visitor#at_gconst env gc in
      super#on_gconst env gc

    method! on_file_attribute env fa =
      let env = visitor#at_file_attribute env fa in
      super#on_file_attribute env fa

    method! on_shape_field_name env sfn =
      let env = visitor#at_shape_field_name env sfn in
      super#on_shape_field_name env sfn

    method! on_user_attribute env ua =
      let env = visitor#at_user_attribute env ua in
      super#on_user_attribute env ua

    method! on_class_id env ci =
      let env = visitor#at_class_id env ci in
      super#on_class_id env ci

    method! on_catch env catch =
      let env = visitor#at_catch env catch in
      super#on_catch env catch

    method! on_targ env targ =
      let env = visitor#at_targ env targ in
      super#on_targ env targ

    method! on_class_hint env ch =
      let env = visitor#at_class_hint env ch in
      super#on_class_hint env ch

    method! on_trait_hint env th =
      let env = visitor#at_trait_hint env th in
      super#on_trait_hint env th

    method! on_record_hint env rh =
      let env = visitor#at_record_hint env rh in
      super#on_record_hint env rh

    method! on_xhp_attr_hint env xhp_attr =
      let env = visitor#at_xhp_attr_hint env xhp_attr in
      super#on_xhp_attr_hint env xhp_attr
  end
