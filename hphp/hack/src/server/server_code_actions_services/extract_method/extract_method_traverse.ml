open Hh_prelude
module T = Extract_method_types

let rec calc_placeholder_name taken_names n =
  let placeholder = "method" ^ string_of_int n in
  if String.Set.mem taken_names placeholder then
    calc_placeholder_name taken_names (n + 1)
  else
    placeholder

(** tracks what variables are defined *)
module Scopes = struct
  type t = String.Set.t list

  let empty = []

  let enter t = String.Set.empty :: t

  let exit t = List.tl t |> Option.value ~default:empty

  let set_defined t var_name =
    match t with
    | hd :: tl -> String.Set.add hd var_name :: tl
    | [] -> [String.Set.singleton var_name]

  let is_defined t var_name =
    List.exists t ~f:(fun vars -> String.Set.mem vars var_name)
end

(**
We have 3 regions:
- pre-selection
- selection
- post-selection

[Region.t] tracks sundry information we collect from a region
*)
module Region = struct
  type t = {
    referenced: T.var_tys;
        (** variables referenced from the region, along with their types *)
    defined: Scopes.t;  (** what variables are defined in the region *)
    iterator_kind: T.iterator_kind option;
    is_async: bool;
    has_return: bool;
        (** When a region contains a `return`, that effects whether/how we can extract a method.
    As of this writing, we do not provide the refactor if the region containing the selection has a `return`.
    *)
  }

  let empty =
    {
      referenced = String.Map.empty;
      defined = Scopes.empty;
      iterator_kind = None;
      is_async = false;
      has_return = false;
    }

  let free { referenced; defined; _ } =
    referenced
    |> String.Map.filter_keys ~f:(fun key ->
           (Fn.non @@ Scopes.is_defined defined) key)

  let used_from ~(defined_in : t) ~(referenced_from : t) =
    referenced_from
    |> free
    |> String.Map.filter_keys ~f:(Scopes.is_defined defined_in.defined)
end

let plus_candidate (a : T.candidate option) (b : T.candidate option) =
  match (a, b) with
  | (Some a, Some b) ->
    let use_first ~key:_ v1 _v2 = v1 in
    Some
      T.
        {
          (* For these fields, just take [b]'s value, since [a] and [b]
             are always the same anyway *)
          method_pos = b.method_pos;
          method_is_static = b.method_is_static;
          placeholder_name = b.placeholder_name;
          selection_kind = b.selection_kind;
          (* We grow `pos` to fill the selection
             and learn more about whether to make an async or static function, etc. *)
          pos = Pos.merge a.pos b.pos;
          is_async = a.is_async || b.is_async;
          (* Note:  if `(a, b)` is `(Some it_kind_a, Some it_kind_b)` then it doesn't matter whether
             we use a or b assuming the user was consistent in yielding values or yielding key=>value pairs.
             If the user is inconsistent then hopefully they don't mind we go with `b`.
          *)
          iterator_kind = Option.first_some b.iterator_kind a.iterator_kind;
          (* `params` are calculated based on variables that are defined in the
             pre-selection region and used in the selection region.
             If the same variable has two different types then something weird is happening. Just use b's type.
          *)
          params = String.Map.merge_skewed a.params b.params ~combine:use_first;
          (* `return` is calculated based on variables that are defined in the
             selection region and used in the post-selection region.
             If the same variable has two different types, then something weird is happening. Just use b's type.
          *)
          return = String.Map.merge_skewed a.return b.return ~combine:use_first;
        }
  | _ -> Option.first_some a b

let positions_visitor
    (selection : Pos.t) ~method_pos ~method_is_static ~method_names =
  (* These refs are used to accumulate context top->down left->right *)
  (*
The region before the user's selection. This is important for finding
the types of variables that are free in the selection, which is used in calculating
the parameter types for the extracted method.
*)
  let pre_selection_region = ref Region.empty in
  (*
The selection region is used for finding which variables
are free in the selection, which is used in calculating parameters for the extracted method.
The selection region is also used for finding the types
of variables that are used in the post-selection region, which is used for calculating
the return of the extracted method.
*)
  let selection_region = ref Region.empty in
  (*
See [selection region]
*)
  let post_selection_region = ref Region.empty in
  (* positions that overlap the selection, used for ensuring that a selection is valid.
     See  [ensure_selection_common_root] *)
  let expr_positions_overlapping_selection = ref [] in

  (* Count of statements in the selection. Used to calculate [selection_kind] *)
  let stmts_in_selection_count = ref 0 in
  (* Whether we are currently in an lvalue. This affects whether a variable is being used or defined. *)
  let in_lvalue = ref false in

  let with_in_lvalue f =
    let orig_in_lvalue = !in_lvalue in
    in_lvalue := true;
    let res = f () in
    in_lvalue := orig_in_lvalue;
    res
  in

  let ensure_selection_common_root : T.candidate option -> T.candidate option =
    (* filter out invalid selection like this:
                  (1 + 2) +  3
                      ^-----^ selection
              and this:
                  $x = 1 + 2; $y = 3;
                  ^--------------^ selection

       We do not offer refactorings for invalid selections. *)
    Option.filter ~f:(fun candidate ->
        List.for_all !expr_positions_overlapping_selection ~f:(fun p ->
            Pos.(contains candidate.T.pos p || contains p candidate.T.pos)))
  in

  let placeholder_name = calc_placeholder_name method_names 0 in

  let current_region pos : Region.t ref =
    if Pos.start_offset selection > Pos.start_offset pos then
      pre_selection_region
    else if Pos.contains selection pos then
      selection_region
    else
      post_selection_region
  in

  let make acc pos ty_string =
    if (not !in_lvalue) && Pos.contains selection pos then
      if !selection_region.Region.has_return then
        None
      else
        let params = Region.free !selection_region in
        let return = String.Map.empty (* adjusted after the selection *) in
        let selection_kind =
          if !stmts_in_selection_count = 0 then
            T.SelectionKindExpression ty_string
          else
            T.SelectionKindStatement
        in
        plus_candidate acc
        @@ Some
             T.
               {
                 pos;
                 method_pos;
                 method_is_static;
                 placeholder_name;
                 selection_kind;
                 params;
                 return;
                 is_async = !selection_region.Region.is_async;
                 iterator_kind = !selection_region.Region.iterator_kind;
               }
    else
      acc
  in

  object (self)
    inherit [T.candidate option] Tast_visitor.reduce as super

    method zero = None

    method plus = plus_candidate

    method! on_method_ env meth =
      super#on_method_ env meth
      |> ensure_selection_common_root
      |> Option.map ~f:(fun acc ->
             T.
               {
                 acc with
                 return =
                   Region.used_from
                     ~defined_in:!selection_region
                     ~referenced_from:!post_selection_region;
               })

    method! on_as_expr env as_expr =
      with_in_lvalue (fun () -> super#on_as_expr env as_expr)

    method! on_stmt env ((pos, stmt_) as stmt) =
      if Pos.contains selection pos then incr stmts_in_selection_count;
      let region = current_region pos in
      (match stmt_ with
      | Aast.Awaitall (tmp_var_block_pairs, _) ->
        tmp_var_block_pairs
        |> List.iter ~f:(function ((_, tmp_lid), _) ->
               let name = Local_id.get_name tmp_lid in
               region :=
                 Region.
                   {
                     !region with
                     defined = Scopes.set_defined !region.defined name;
                   })
      | Aast.Foreach (_, as_, _) ->
        let (iterator_kind, is_async) =
          Aast.(
            match as_ with
            | As_v _ -> (Some T.Iterator, false)
            | Await_as_v _ -> (Some T.KeyedIterator, true)
            | As_kv _ -> (Some T.KeyedIterator, false)
            | Await_as_kv _ -> (Some T.KeyedIterator, true))
        in
        region := Region.{ !region with iterator_kind; is_async }
      | Aast.Return _ -> region := Region.{ !region with has_return = true }
      | _ -> ());
      let acc = super#on_stmt env stmt in
      let ty = Typing_make_type.void Typing_reason.Rnone in
      make acc pos (Code_action_types.Type_string.of_locl_ty env ty)

    method! on_fun_ env fun_ =
      let open Aast_defs in
      let region = current_region fun_.f_span in
      let add_param { param_name; _ } =
        region :=
          Region.
            {
              !region with
              defined = Scopes.set_defined !region.defined param_name;
            }
      in
      fun_.f_params |> List.iter ~f:add_param;
      super#on_fun_ env fun_

    method! on_expr env expr =
      let (ty, pos, expr_) = expr in
      let ty_string = Code_action_types.Type_string.of_locl_ty env ty in
      if Pos.overlaps selection pos then
        expr_positions_overlapping_selection :=
          pos :: !expr_positions_overlapping_selection;
      let region = current_region pos in
      let acc =
        (* mutates refs *)
        match expr_ with
        | Aast.Lfun (fun_, _)
        | Aast.Efun Aast.{ ef_fun = fun_; _ } ->
          (region :=
             Region.{ !region with defined = Scopes.enter !region.defined });
          let acc =
            match Aast_defs.(fun_.f_body.fb_ast) with
            | [(_, Aast.Return (Some e))] ->
              (* `() ==> 3 + 3` has a `return` in the tast, which we remove because "extract method" isn't
                 safe for return statements in general. But it's fine to offer the refactor this case: we can extract the `3 + 3`.
              *)
              super#on_expr env e
            | _ -> super#on_expr_ env expr_
          in
          (region :=
             Region.{ !region with defined = Scopes.exit !region.defined });
          acc
        | Aast.(Binop { bop = Ast_defs.Eq _; lhs; rhs }) ->
          let rhs = self#on_expr env rhs in
          self#plus rhs (with_in_lvalue (fun () -> self#on_expr env lhs))
        | Aast.Lvar (_, lid) when !in_lvalue ->
          let (_ : T.candidate option) = super#on_expr env expr in
          (region :=
             Region.
               {
                 !region with
                 defined =
                   Scopes.set_defined !region.defined (Local_id.get_name lid);
               });
          None
        | Aast.Lvar (_, lid) ->
          let name = Local_id.get_name lid in
          Region.(
            if not @@ Scopes.is_defined !region.defined name then
              region :=
                {
                  !region with
                  referenced =
                    String.Map.set ~key:name ~data:ty_string !region.referenced;
                });
          super#on_expr env expr
        | Aast.Yield af ->
          let iterator_kind =
            Aast.(
              match af with
              | AFvalue _ -> Some T.Iterator
              | AFkvalue _ -> Some T.KeyedIterator)
          in
          (region := Region.{ !region with iterator_kind });
          super#on_expr env expr
        | Aast.Await _ ->
          (region := Region.{ !region with is_async = true });
          super#on_expr env expr
        | _ -> super#on_expr env expr
      in
      make acc pos ty_string
  end

(**
- Avoid traversing methods outside the selection
- Pass information [positions_visitor] needs
 *)
let top_visitor ~(selection : Pos.t) =
  object (self)
    inherit [_] Tast_visitor.reduce

    method zero = None

    method plus = Option.first_some

    method! on_def env =
      function
      | Aast.Class class_ when Pos.contains class_.Aast.c_span selection ->
        self#on_class_ env class_
      | _ -> None

    method! on_class_ env class_ =
      let methods = class_.Aast.c_methods in
      let method_names =
        methods
        |> List.map ~f:(fun Aast.{ m_name; _ } -> snd m_name)
        |> String.Set.of_list
      in
      methods
      |> List.find ~f:(fun Aast.{ m_span; _ } -> Pos.contains m_span selection)
      |> Option.bind ~f:(fun meth ->
             let visitor =
               positions_visitor
                 selection
                 ~method_pos:meth.Aast.m_span
                 ~method_is_static:meth.Aast.m_static
                 ~method_names
             in
             visitor#on_method_ env meth)
  end

let find_candidate ~selection ~entry ctx =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in
  (top_visitor ~selection)#go
    ctx
    tast.Tast_with_dynamic.under_normal_assumptions
