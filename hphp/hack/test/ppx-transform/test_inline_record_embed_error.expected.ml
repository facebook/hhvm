module Inline_record_error : sig
  type t =
    | Inline_record of {
        a: int;
        b: t option;
      } [@transform.explicit]
    | Other of t option
  [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

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
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type t =
    | Inline_record of {
        a: int;
        b: t option;
      } [@transform.explicit]
    | Other of t option
  [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : t) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_t:
          (t ->
          ctx:'ctx ->
          'ctx * [ `Stop of t | `Continue of t | `Restart of t ])
          option;
      }

      let identity _ = { on_ty_t = None }

      let _ = identity

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

      let _ = combine
    end

    let rec (traverse :
              t ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              t) =
     fun t ~ctx ~top_down ~bottom_up ->
      match t with
      | Inline_record ({ b; _ } as inline_record) ->
        [%ocaml.error
          "Explicit transforms of inline records are not supported by the `ppx_transform` preprocessor"]
      | Other other_elem ->
        Other
          (match other_elem with
          | Some other_elem_inner ->
            Some (transform other_elem_inner ~ctx ~top_down ~bottom_up)
          | _ -> None)

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

    let _ = traverse

    and _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end
