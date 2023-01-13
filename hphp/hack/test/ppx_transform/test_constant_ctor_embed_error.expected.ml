module Constant_ctor_error : sig
  type t =
    | Constant [@deriving transform.explicit]
    | Other of t option
  [@@deriving transform]

  module Pass : sig
    type nonrec 'ctx t = {
      on_ty_t:
        (t ->
        ctx:'ctx ->
        'ctx * [ `Stop of t | `Continue of t | `Restart of t ])
        option;
    }

    val combine : 'ctx t -> 'ctx t -> 'ctx t

    val identity : unit -> 'ctx t
  end

  val transform :
    t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t
end = struct
  type t =
    | Constant [@deriving transform.explicit]
    | Other of t option
  [@@deriving transform]

  module Pass = struct
    type nonrec 'ctx t = {
      on_ty_t:
        (t ->
        ctx:'ctx ->
        'ctx * [ `Stop of t | `Continue of t | `Restart of t ])
        option;
    }

    let identity _ = { on_ty_t = None }

    let combine p1 p2 =
      {
        on_ty_t =
          (match (p1.on_ty_t, p2.on_ty_t) with
          | (Some t1, Some t2) ->
            Some
              (fun elem ~ctx ->
                match t1 elem ~ctx with
                | (ctx, `Continue elem) -> t2 elem ~ctx
                | otherwise -> otherwise)
          | (None, _) -> p2.on_ty_t
          | _ -> p1.on_ty_t);
      }
  end

  let rec (traverse :
            t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t)
      =
   fun t ~ctx ~top_down ~bottom_up ->
    match t with
    | Other other_elem ->
      Other
        (match other_elem with
        | Some other_elem_inner ->
          Some (transform other_elem_inner ~ctx ~top_down ~bottom_up)
        | _ -> None)
    | t -> t

  and (transform :
        t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t) =
   fun elem ~ctx ~top_down ~bottom_up ->
    let initial_ctx = ctx in
    match top_down.Pass.on_ty_t with
    | Some td ->
      (match td elem ~ctx with
      | (_ctx, `Stop elem) -> elem
      | (ctx, `Continue elem) ->
        let elem = traverse elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_t with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform elem ~ctx:initial_ctx ~top_down ~bottom_up))
      | (_ctx, `Restart elem) ->
        transform elem ~ctx:initial_ctx ~top_down ~bottom_up)
    | _ ->
      let elem = traverse elem ~ctx ~top_down ~bottom_up in
      (match bottom_up.Pass.on_ty_t with
      | None -> elem
      | Some bu ->
        (match bu elem ~ctx with
        | (_ctx, (`Continue elem | `Stop elem)) -> elem
        | (_ctx, `Restart elem) ->
          transform elem ~ctx:initial_ctx ~top_down ~bottom_up))
end
