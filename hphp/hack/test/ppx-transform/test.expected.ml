module Variant : sig
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
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
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
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
      | Plus (plus_elem_0, plus_elem_1) ->
        Plus
          ( transform plus_elem_0 ~ctx ~top_down ~bottom_up,
            transform plus_elem_1 ~ctx ~top_down ~bottom_up )
      | Leq (leq_elem_0, leq_elem_1) ->
        Leq
          ( transform leq_elem_0 ~ctx ~top_down ~bottom_up,
            transform leq_elem_1 ~ctx ~top_down ~bottom_up )
      | Cond (cond_elem_0, cond_elem_1, cond_elem_2) ->
        Cond
          ( transform cond_elem_0 ~ctx ~top_down ~bottom_up,
            transform cond_elem_1 ~ctx ~top_down ~bottom_up,
            transform cond_elem_2 ~ctx ~top_down ~bottom_up )
      | t -> t

    and (transform :
          t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_t with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_t with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

    let _ = traverse

    and _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Gadt : sig
  type _ t =
    | T : bool t
    | F : bool t
    | Num : int -> int t
    | Plus : (int t * int t) -> int t
    | Leq : (int t * int t) -> bool t
    | Cond : (bool t * 'a t * 'a t) -> 'a t
  [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform :
      'a t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> 'a t
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type _ t =
    | T : bool t
    | F : bool t
    | Num : int -> int t
    | Plus : (int t * int t) -> int t
    | Leq : (int t * int t) -> bool t
    | Cond : (bool t * 'a t * 'a t) -> 'a t
  [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : _ t) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
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

    let rec traverse :
              'a.
              'a t ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a t =
      fun (type a) (t : a t) ~ctx ~top_down ~bottom_up : a t ->
       match t with
       | Plus (plus_elem_0, plus_elem_1) ->
         Plus
           ( transform plus_elem_0 ~ctx ~top_down ~bottom_up,
             transform plus_elem_1 ~ctx ~top_down ~bottom_up )
       | Leq (leq_elem_0, leq_elem_1) ->
         Leq
           ( transform leq_elem_0 ~ctx ~top_down ~bottom_up,
             transform leq_elem_1 ~ctx ~top_down ~bottom_up )
       | Cond (cond_elem_0, cond_elem_1, cond_elem_2) ->
         Cond
           ( transform cond_elem_0 ~ctx ~top_down ~bottom_up,
             transform cond_elem_1 ~ctx ~top_down ~bottom_up,
             transform cond_elem_2 ~ctx ~top_down ~bottom_up )
       | t -> t

    and transform :
          'a.
          'a t ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a t =
      fun (type a) (elem : a t) ~ctx ~top_down ~bottom_up : a t ->
       match top_down.Pass.on_ty_t with
       | Some td ->
         (match td elem ~ctx with
         | (_ctx, `Stop elem) -> elem
         | (td_ctx, `Continue elem) ->
           let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
           (match bottom_up.Pass.on_ty_t with
           | None -> elem
           | Some bu ->
             (match bu elem ~ctx with
             | (_ctx, (`Continue elem | `Stop elem)) -> elem
             | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))
         | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
       | _ ->
         let elem = traverse elem ~ctx ~top_down ~bottom_up in
         (match bottom_up.Pass.on_ty_t with
         | None -> elem
         | Some bu ->
           (match bu elem ~ctx with
           | (_ctx, (`Continue elem | `Stop elem)) -> elem
           | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

    let _ = traverse

    and _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Nonregular : sig
  type 'a term =
    | Var of 'a
    | App of 'a term * 'a term
    | Abs of 'a bind term

  and 'a bind =
    | Zero
    | Succ of 'a
  [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_bind:
          'a.
          ('a bind ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a bind | `Continue of 'a bind | `Restart of 'a bind ])
          option;
        on_ty_term:
          'a.
          ('a term ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a term | `Continue of 'a term | `Restart of 'a term ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform_ty_bind :
      'a bind ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      'a bind

    val transform_ty_term :
      'a term ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      'a term
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type 'a term =
    | Var of 'a
    | App of 'a term * 'a term
    | Abs of 'a bind term

  and 'a bind =
    | Zero
    | Succ of 'a
  [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : 'a term) -> ())

    let _ = (fun (_ : 'a bind) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_bind:
          'a.
          ('a bind ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a bind | `Continue of 'a bind | `Restart of 'a bind ])
          option;
        on_ty_term:
          'a.
          ('a term ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a term | `Continue of 'a term | `Restart of 'a term ])
          option;
      }

      let identity _ = { on_ty_bind = None; on_ty_term = None }

      let _ = identity

      let combine p1 p2 =
        {
          on_ty_bind =
            (match (p1.on_ty_bind, p2.on_ty_bind) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_bind
            | _ -> p1.on_ty_bind);
          on_ty_term =
            (match (p1.on_ty_term, p2.on_ty_term) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_term
            | _ -> p1.on_ty_term);
        }

      let _ = combine
    end

    let rec transform_ty_bind :
              'a.
              'a bind ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a bind =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_bind with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (_ctx, `Continue elem) ->
          (match bottom_up.Pass.on_ty_bind with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_bind elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_bind elem ~ctx ~top_down ~bottom_up)
      | _ ->
        (match bottom_up.Pass.on_ty_bind with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_bind elem ~ctx ~top_down ~bottom_up))

    let _ = transform_ty_bind

    let rec traverse_ty_term :
              'a.
              'a term ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a term =
     fun term ~ctx ~top_down ~bottom_up ->
      match term with
      | App (app_elem_0, app_elem_1) ->
        App
          ( transform_ty_term app_elem_0 ~ctx ~top_down ~bottom_up,
            transform_ty_term app_elem_1 ~ctx ~top_down ~bottom_up )
      | Abs abs_elem ->
        Abs (transform_ty_term abs_elem ~ctx ~top_down ~bottom_up)
      | term -> term

    and transform_ty_term :
          'a.
          'a term ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a term =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_term with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_term elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_term with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_term elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_term elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_term elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_term with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_term elem ~ctx ~top_down ~bottom_up))

    let _ = traverse_ty_term

    and _ = transform_ty_term
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Nonregular_mutual : sig
  type 'a one =
    | Nil
    | Two of 'a two

  and 'a two = MaybeOne of 'a option one [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_two:
          'a.
          ('a two ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a two | `Continue of 'a two | `Restart of 'a two ])
          option;
        on_ty_one:
          'a.
          ('a one ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a one | `Continue of 'a one | `Restart of 'a one ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform_ty_two :
      'a two ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      'a two

    val transform_ty_one :
      'a one ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      'a one
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type 'a one =
    | Nil
    | Two of 'a two

  and 'a two = MaybeOne of 'a option one [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : 'a one) -> ())

    let _ = (fun (_ : 'a two) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_two:
          'a.
          ('a two ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a two | `Continue of 'a two | `Restart of 'a two ])
          option;
        on_ty_one:
          'a.
          ('a one ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a one | `Continue of 'a one | `Restart of 'a one ])
          option;
      }

      let identity _ = { on_ty_two = None; on_ty_one = None }

      let _ = identity

      let combine p1 p2 =
        {
          on_ty_two =
            (match (p1.on_ty_two, p2.on_ty_two) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_two
            | _ -> p1.on_ty_two);
          on_ty_one =
            (match (p1.on_ty_one, p2.on_ty_one) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_one
            | _ -> p1.on_ty_one);
        }

      let _ = combine
    end

    let rec traverse_ty_two :
              'a.
              'a two ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a two =
     fun two ~ctx ~top_down ~bottom_up ->
      match two with
      | MaybeOne maybeone_elem ->
        MaybeOne (transform_ty_one maybeone_elem ~ctx ~top_down ~bottom_up)

    and transform_ty_two :
          'a.
          'a two ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a two =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_two with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_two elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_two with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_two elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_two elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_two elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_two with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_two elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_one :
          'a.
          'a one ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a one =
     fun one ~ctx ~top_down ~bottom_up ->
      match one with
      | Two two_elem ->
        Two (transform_ty_two two_elem ~ctx ~top_down ~bottom_up)
      | one -> one

    and transform_ty_one :
          'a.
          'a one ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a one =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_one with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_one elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_one with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_one elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_one elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_one elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_one with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_one elem ~ctx ~top_down ~bottom_up))

    let _ = traverse_ty_two

    and _ = transform_ty_two

    and _ = traverse_ty_one

    and _ = transform_ty_one
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Nonregular_mutual_gadt : sig
  type 'a one =
    | Nil : 'a one
    | Two : 'a two -> 'a one

  and 'a two = MaybeOne of 'a option one [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_two:
          'a.
          ('a two ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a two | `Continue of 'a two | `Restart of 'a two ])
          option;
        on_ty_one:
          'a.
          ('a one ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a one | `Continue of 'a one | `Restart of 'a one ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform_ty_two :
      'a two ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      'a two

    val transform_ty_one :
      'a one ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      'a one
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type 'a one =
    | Nil : 'a one
    | Two : 'a two -> 'a one

  and 'a two = MaybeOne of 'a option one [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : 'a one) -> ())

    let _ = (fun (_ : 'a two) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_two:
          'a.
          ('a two ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a two | `Continue of 'a two | `Restart of 'a two ])
          option;
        on_ty_one:
          'a.
          ('a one ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a one | `Continue of 'a one | `Restart of 'a one ])
          option;
      }

      let identity _ = { on_ty_two = None; on_ty_one = None }

      let _ = identity

      let combine p1 p2 =
        {
          on_ty_two =
            (match (p1.on_ty_two, p2.on_ty_two) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_two
            | _ -> p1.on_ty_two);
          on_ty_one =
            (match (p1.on_ty_one, p2.on_ty_one) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_one
            | _ -> p1.on_ty_one);
        }

      let _ = combine
    end

    let rec traverse_ty_two :
              'a.
              'a two ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a two =
     fun two ~ctx ~top_down ~bottom_up ->
      match two with
      | MaybeOne maybeone_elem ->
        MaybeOne (transform_ty_one maybeone_elem ~ctx ~top_down ~bottom_up)

    and transform_ty_two :
          'a.
          'a two ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a two =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_two with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_two elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_two with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_two elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_two elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_two elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_two with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_two elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_one :
          'a.
          'a one ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a one =
      fun (type a) (one : a one) ~ctx ~top_down ~bottom_up : a one ->
       match one with
       | Two two_elem ->
         Two (transform_ty_two two_elem ~ctx ~top_down ~bottom_up)
       | one -> one

    and transform_ty_one :
          'a.
          'a one ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a one =
      fun (type a) (elem : a one) ~ctx ~top_down ~bottom_up : a one ->
       match top_down.Pass.on_ty_one with
       | Some td ->
         (match td elem ~ctx with
         | (_ctx, `Stop elem) -> elem
         | (td_ctx, `Continue elem) ->
           let elem = traverse_ty_one elem ~ctx:td_ctx ~top_down ~bottom_up in
           (match bottom_up.Pass.on_ty_one with
           | None -> elem
           | Some bu ->
             (match bu elem ~ctx with
             | (_ctx, (`Continue elem | `Stop elem)) -> elem
             | (_ctx, `Restart elem) ->
               transform_ty_one elem ~ctx ~top_down ~bottom_up))
         | (_ctx, `Restart elem) ->
           transform_ty_one elem ~ctx ~top_down ~bottom_up)
       | _ ->
         let elem = traverse_ty_one elem ~ctx ~top_down ~bottom_up in
         (match bottom_up.Pass.on_ty_one with
         | None -> elem
         | Some bu ->
           (match bu elem ~ctx with
           | (_ctx, (`Continue elem | `Stop elem)) -> elem
           | (_ctx, `Restart elem) ->
             transform_ty_one elem ~ctx ~top_down ~bottom_up))

    let _ = traverse_ty_two

    and _ = transform_ty_two

    and _ = traverse_ty_one

    and _ = transform_ty_one
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Composed = struct
  module BinOp : sig
    type t =
      | Add
      | Sub
      | Mul
      | Div
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
      | Add
      | Sub
      | Mul
      | Div
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

      let rec (transform :
                t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                t) =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (_ctx, `Continue elem) ->
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module RelOp : sig
    type t =
      | Eq
      | Lt
      | Gt
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
      | Eq
      | Lt
      | Gt
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

      let rec (transform :
                t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                t) =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (_ctx, `Continue elem) ->
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module ConnOp : sig
    type t =
      | And
      | Or
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
      | And
      | Or
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

      let rec (transform :
                t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                t) =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (_ctx, `Continue elem) ->
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module Expr : sig
    type t =
      | Num of int
      | Bin of BinOp.t * t * t
      | Rel of RelOp.t * t * t
      | Conn of ConnOp.t * t * t
      | Cond of t * t * t
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
          on_BinOp: 'ctx BinOp.Pass.t option;
          on_ConnOp: 'ctx ConnOp.Pass.t option;
          on_RelOp: 'ctx RelOp.Pass.t option;
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
      | Num of int
      | Bin of BinOp.t * t * t
      | Rel of RelOp.t * t * t
      | Conn of ConnOp.t * t * t
      | Cond of t * t * t
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
          on_BinOp: 'ctx BinOp.Pass.t option;
          on_ConnOp: 'ctx ConnOp.Pass.t option;
          on_RelOp: 'ctx RelOp.Pass.t option;
        }

        let identity _ =
          { on_ty_t = None; on_BinOp = None; on_ConnOp = None; on_RelOp = None }

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
            on_BinOp =
              (match (p1.on_BinOp, p2.on_BinOp) with
              | (Some p1, Some p2) -> Some (BinOp.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_BinOp
              | _ -> p2.on_BinOp);
            on_ConnOp =
              (match (p1.on_ConnOp, p2.on_ConnOp) with
              | (Some p1, Some p2) -> Some (ConnOp.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_ConnOp
              | _ -> p2.on_ConnOp);
            on_RelOp =
              (match (p1.on_RelOp, p2.on_RelOp) with
              | (Some p1, Some p2) -> Some (RelOp.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_RelOp
              | _ -> p2.on_RelOp);
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
        | Bin (bin_elem_0, bin_elem_1, bin_elem_2) ->
          Bin
            ( (match (top_down.Pass.on_BinOp, bottom_up.Pass.on_BinOp) with
              | (Some top_down, Some bottom_up) ->
                BinOp.transform bin_elem_0 ~ctx ~top_down ~bottom_up
              | (Some top_down, _) ->
                BinOp.transform
                  bin_elem_0
                  ~ctx
                  ~top_down
                  ~bottom_up:(BinOp.Pass.identity ())
              | (_, Some bottom_up) ->
                BinOp.transform
                  bin_elem_0
                  ~ctx
                  ~top_down:(BinOp.Pass.identity ())
                  ~bottom_up
              | _ -> bin_elem_0),
              transform bin_elem_1 ~ctx ~top_down ~bottom_up,
              transform bin_elem_2 ~ctx ~top_down ~bottom_up )
        | Rel (rel_elem_0, rel_elem_1, rel_elem_2) ->
          Rel
            ( (match (top_down.Pass.on_RelOp, bottom_up.Pass.on_RelOp) with
              | (Some top_down, Some bottom_up) ->
                RelOp.transform rel_elem_0 ~ctx ~top_down ~bottom_up
              | (Some top_down, _) ->
                RelOp.transform
                  rel_elem_0
                  ~ctx
                  ~top_down
                  ~bottom_up:(RelOp.Pass.identity ())
              | (_, Some bottom_up) ->
                RelOp.transform
                  rel_elem_0
                  ~ctx
                  ~top_down:(RelOp.Pass.identity ())
                  ~bottom_up
              | _ -> rel_elem_0),
              transform rel_elem_1 ~ctx ~top_down ~bottom_up,
              transform rel_elem_2 ~ctx ~top_down ~bottom_up )
        | Conn (conn_elem_0, conn_elem_1, conn_elem_2) ->
          Conn
            ( (match (top_down.Pass.on_ConnOp, bottom_up.Pass.on_ConnOp) with
              | (Some top_down, Some bottom_up) ->
                ConnOp.transform conn_elem_0 ~ctx ~top_down ~bottom_up
              | (Some top_down, _) ->
                ConnOp.transform
                  conn_elem_0
                  ~ctx
                  ~top_down
                  ~bottom_up:(ConnOp.Pass.identity ())
              | (_, Some bottom_up) ->
                ConnOp.transform
                  conn_elem_0
                  ~ctx
                  ~top_down:(ConnOp.Pass.identity ())
                  ~bottom_up
              | _ -> conn_elem_0),
              transform conn_elem_1 ~ctx ~top_down ~bottom_up,
              transform conn_elem_2 ~ctx ~top_down ~bottom_up )
        | Cond (cond_elem_0, cond_elem_1, cond_elem_2) ->
          Cond
            ( transform cond_elem_0 ~ctx ~top_down ~bottom_up,
              transform cond_elem_1 ~ctx ~top_down ~bottom_up,
              transform cond_elem_2 ~ctx ~top_down ~bottom_up )
        | t -> t

      and (transform :
            t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t)
          =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (td_ctx, `Continue elem) ->
            let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          let elem = traverse elem ~ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module Expr_gadt : sig
    type _ t =
      | Num : int -> int t
      | Bin : (BinOp.t * int t * int t) -> int t
      | Rel : (RelOp.t * int t * int t) -> bool t
      | Conn : (ConnOp.t * bool t * bool t) -> bool t
      | Cond : (bool t * 'a t * 'a t) -> 'a t
    [@@deriving transform]

    include sig
      [@@@ocaml.warning "-32-60"]

      module Pass : sig
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
            option;
          on_BinOp: 'ctx BinOp.Pass.t option;
          on_ConnOp: 'ctx ConnOp.Pass.t option;
          on_RelOp: 'ctx RelOp.Pass.t option;
        }

        val combine : 'ctx t -> 'ctx t -> 'ctx t

        val identity : unit -> 'ctx t
      end

      val transform :
        'a t ->
        ctx:'ctx ->
        top_down:'ctx Pass.t ->
        bottom_up:'ctx Pass.t ->
        'a t
    end
    [@@ocaml.doc "@inline"] [@@merlin.hide]
  end = struct
    type _ t =
      | Num : int -> int t
      | Bin : (BinOp.t * int t * int t) -> int t
      | Rel : (RelOp.t * int t * int t) -> bool t
      | Conn : (ConnOp.t * bool t * bool t) -> bool t
      | Cond : (bool t * 'a t * 'a t) -> 'a t
    [@@deriving transform]

    include struct
      [@@@ocaml.warning "-60"]

      let _ = (fun (_ : _ t) -> ())

      module Pass = struct
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
            option;
          on_BinOp: 'ctx BinOp.Pass.t option;
          on_ConnOp: 'ctx ConnOp.Pass.t option;
          on_RelOp: 'ctx RelOp.Pass.t option;
        }

        let identity _ =
          { on_ty_t = None; on_BinOp = None; on_ConnOp = None; on_RelOp = None }

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
            on_BinOp =
              (match (p1.on_BinOp, p2.on_BinOp) with
              | (Some p1, Some p2) -> Some (BinOp.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_BinOp
              | _ -> p2.on_BinOp);
            on_ConnOp =
              (match (p1.on_ConnOp, p2.on_ConnOp) with
              | (Some p1, Some p2) -> Some (ConnOp.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_ConnOp
              | _ -> p2.on_ConnOp);
            on_RelOp =
              (match (p1.on_RelOp, p2.on_RelOp) with
              | (Some p1, Some p2) -> Some (RelOp.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_RelOp
              | _ -> p2.on_RelOp);
          }

        let _ = combine
      end

      let rec traverse :
                'a.
                'a t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                'a t =
        fun (type a) (t : a t) ~ctx ~top_down ~bottom_up : a t ->
         match t with
         | Bin (bin_elem_0, bin_elem_1, bin_elem_2) ->
           Bin
             ( (match (top_down.Pass.on_BinOp, bottom_up.Pass.on_BinOp) with
               | (Some top_down, Some bottom_up) ->
                 BinOp.transform bin_elem_0 ~ctx ~top_down ~bottom_up
               | (Some top_down, _) ->
                 BinOp.transform
                   bin_elem_0
                   ~ctx
                   ~top_down
                   ~bottom_up:(BinOp.Pass.identity ())
               | (_, Some bottom_up) ->
                 BinOp.transform
                   bin_elem_0
                   ~ctx
                   ~top_down:(BinOp.Pass.identity ())
                   ~bottom_up
               | _ -> bin_elem_0),
               transform bin_elem_1 ~ctx ~top_down ~bottom_up,
               transform bin_elem_2 ~ctx ~top_down ~bottom_up )
         | Rel (rel_elem_0, rel_elem_1, rel_elem_2) ->
           Rel
             ( (match (top_down.Pass.on_RelOp, bottom_up.Pass.on_RelOp) with
               | (Some top_down, Some bottom_up) ->
                 RelOp.transform rel_elem_0 ~ctx ~top_down ~bottom_up
               | (Some top_down, _) ->
                 RelOp.transform
                   rel_elem_0
                   ~ctx
                   ~top_down
                   ~bottom_up:(RelOp.Pass.identity ())
               | (_, Some bottom_up) ->
                 RelOp.transform
                   rel_elem_0
                   ~ctx
                   ~top_down:(RelOp.Pass.identity ())
                   ~bottom_up
               | _ -> rel_elem_0),
               transform rel_elem_1 ~ctx ~top_down ~bottom_up,
               transform rel_elem_2 ~ctx ~top_down ~bottom_up )
         | Conn (conn_elem_0, conn_elem_1, conn_elem_2) ->
           Conn
             ( (match (top_down.Pass.on_ConnOp, bottom_up.Pass.on_ConnOp) with
               | (Some top_down, Some bottom_up) ->
                 ConnOp.transform conn_elem_0 ~ctx ~top_down ~bottom_up
               | (Some top_down, _) ->
                 ConnOp.transform
                   conn_elem_0
                   ~ctx
                   ~top_down
                   ~bottom_up:(ConnOp.Pass.identity ())
               | (_, Some bottom_up) ->
                 ConnOp.transform
                   conn_elem_0
                   ~ctx
                   ~top_down:(ConnOp.Pass.identity ())
                   ~bottom_up
               | _ -> conn_elem_0),
               transform conn_elem_1 ~ctx ~top_down ~bottom_up,
               transform conn_elem_2 ~ctx ~top_down ~bottom_up )
         | Cond (cond_elem_0, cond_elem_1, cond_elem_2) ->
           Cond
             ( transform cond_elem_0 ~ctx ~top_down ~bottom_up,
               transform cond_elem_1 ~ctx ~top_down ~bottom_up,
               transform cond_elem_2 ~ctx ~top_down ~bottom_up )
         | t -> t

      and transform :
            'a.
            'a t ->
            ctx:'ctx ->
            top_down:'ctx Pass.t ->
            bottom_up:'ctx Pass.t ->
            'a t =
        fun (type a) (elem : a t) ~ctx ~top_down ~bottom_up : a t ->
         match top_down.Pass.on_ty_t with
         | Some td ->
           (match td elem ~ctx with
           | (_ctx, `Stop elem) -> elem
           | (td_ctx, `Continue elem) ->
             let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
             (match bottom_up.Pass.on_ty_t with
             | None -> elem
             | Some bu ->
               (match bu elem ~ctx with
               | (_ctx, (`Continue elem | `Stop elem)) -> elem
               | (_ctx, `Restart elem) ->
                 transform elem ~ctx ~top_down ~bottom_up))
           | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
         | _ ->
           let elem = traverse elem ~ctx ~top_down ~bottom_up in
           (match bottom_up.Pass.on_ty_t with
           | None -> elem
           | Some bu ->
             (match bu elem ~ctx with
             | (_ctx, (`Continue elem | `Stop elem)) -> elem
             | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end
end

module Builtins : sig
  type 'a t = {
    prim_ignored: char;
    ref: 'a t ref;
    opt: 'a t option;
    res: ('a t, 'a t) result;
    list: 'a t list;
    array: 'a t array;
    lazy_: 'a t lazy_t;
    nested: ('a t option list, 'a t array option) result list option lazy_t;
  }
  [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform :
      'a t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> 'a t
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type 'a t = {
    prim_ignored: char;
    ref: 'a t ref;
    opt: 'a t option;
    res: ('a t, 'a t) result;
    list: 'a t list;
    array: 'a t array;
    lazy_: 'a t lazy_t;
    nested: ('a t option list, 'a t array option) result list option lazy_t;
  }
  [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : 'a t) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
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

    let rec traverse :
              'a.
              'a t ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a t =
     fun ({ ref; opt; res; list; array; lazy_; nested; _ } as t)
         ~ctx
         ~top_down
         ~bottom_up ->
      {
        t with
        ref =
          (let ref_deref = !ref in
           ref := transform ref_deref ~ctx ~top_down ~bottom_up;
           ref);
        opt =
          (match opt with
          | Some opt_inner ->
            Some (transform opt_inner ~ctx ~top_down ~bottom_up)
          | _ -> None);
        res =
          (match res with
          | Ok res_ok -> Ok (transform res_ok ~ctx ~top_down ~bottom_up)
          | Error res_err -> Error (transform res_err ~ctx ~top_down ~bottom_up));
        list =
          Stdlib.List.map
            (fun list -> transform list ~ctx ~top_down ~bottom_up)
            list;
        array =
          Stdlib.Array.map
            (fun array -> transform array ~ctx ~top_down ~bottom_up)
            array;
        lazy_ =
          (let lazy__force = Lazy.force lazy_ in
           lazy (transform lazy__force ~ctx ~top_down ~bottom_up));
        nested =
          (let nested_force = Lazy.force nested in
           lazy
             (match nested_force with
             | Some nested_force_inner ->
               Some
                 (Stdlib.List.map
                    (fun nested_force_inner ->
                      match nested_force_inner with
                      | Ok nested_force_inner_ok ->
                        Ok
                          (Stdlib.List.map
                             (fun nested_force_inner_ok ->
                               match nested_force_inner_ok with
                               | Some nested_force_inner_ok_inner ->
                                 Some
                                   (transform
                                      nested_force_inner_ok_inner
                                      ~ctx
                                      ~top_down
                                      ~bottom_up)
                               | _ -> None)
                             nested_force_inner_ok)
                      | Error nested_force_inner_err ->
                        Error
                          (match nested_force_inner_err with
                          | Some nested_force_inner_err_inner ->
                            Some
                              (Stdlib.Array.map
                                 (fun nested_force_inner_err_inner ->
                                   transform
                                     nested_force_inner_err_inner
                                     ~ctx
                                     ~top_down
                                     ~bottom_up)
                                 nested_force_inner_err_inner)
                          | _ -> None))
                    nested_force_inner)
             | _ -> None));
      }

    and transform :
          'a.
          'a t ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a t =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_t with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_t with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

    let _ = traverse

    and _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Opaque_annotations : sig
  type variant =
    | Opaque_ctor of record [@transform.opaque]
    | Normal of record
    | Tuple_with_opaque of (record * (alias[@transform.opaque]) * opaque_decl)

  and record = {
    normal: alias;
    opaque: alias; [@transform.opaque]
  }

  and alias = record * (variant[@transform.opaque]) * opaque_decl

  and opaque_decl = Opaque of variant * record * alias
  [@@transform.opaque] [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_alias:
          (alias ->
          ctx:'ctx ->
          'ctx * [ `Stop of alias | `Continue of alias | `Restart of alias ])
          option;
        on_ty_record:
          (record ->
          ctx:'ctx ->
          'ctx * [ `Stop of record | `Continue of record | `Restart of record ])
          option;
        on_ty_variant:
          (variant ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of variant | `Continue of variant | `Restart of variant ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform_ty_alias :
      alias ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      alias

    val transform_ty_record :
      record ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      record

    val transform_ty_variant :
      variant ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      variant
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type variant =
    | Opaque_ctor of record [@transform.opaque]
    | Normal of record
    | Tuple_with_opaque of (record * (alias[@transform.opaque]) * opaque_decl)

  and record = {
    normal: alias;
    opaque: alias; [@transform.opaque]
  }

  and alias = record * (variant[@transform.opaque]) * opaque_decl

  and opaque_decl = Opaque of variant * record * alias
  [@@transform.opaque] [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : variant) -> ())

    let _ = (fun (_ : record) -> ())

    let _ = (fun (_ : alias) -> ())

    let _ = (fun (_ : opaque_decl) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_alias:
          (alias ->
          ctx:'ctx ->
          'ctx * [ `Stop of alias | `Continue of alias | `Restart of alias ])
          option;
        on_ty_record:
          (record ->
          ctx:'ctx ->
          'ctx * [ `Stop of record | `Continue of record | `Restart of record ])
          option;
        on_ty_variant:
          (variant ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of variant | `Continue of variant | `Restart of variant ])
          option;
      }

      let identity _ =
        { on_ty_alias = None; on_ty_record = None; on_ty_variant = None }

      let _ = identity

      let combine p1 p2 =
        {
          on_ty_alias =
            (match (p1.on_ty_alias, p2.on_ty_alias) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_alias
            | _ -> p1.on_ty_alias);
          on_ty_record =
            (match (p1.on_ty_record, p2.on_ty_record) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_record
            | _ -> p1.on_ty_record);
          on_ty_variant =
            (match (p1.on_ty_variant, p2.on_ty_variant) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_variant
            | _ -> p1.on_ty_variant);
        }

      let _ = combine
    end

    let rec (traverse_ty_alias :
              alias ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              alias) =
     fun (alias_0, alias_1, alias_2) ~ctx ~top_down ~bottom_up ->
      (transform_ty_record alias_0 ~ctx ~top_down ~bottom_up, alias_1, alias_2)

    and (transform_ty_alias :
          alias ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          alias) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_alias with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_alias elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_alias with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_alias elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_alias elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_alias elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_alias with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_alias elem ~ctx ~top_down ~bottom_up))

    and (traverse_ty_record :
          record ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          record) =
     fun ({ normal; _ } as record) ~ctx ~top_down ~bottom_up ->
      {
        record with
        normal = transform_ty_alias normal ~ctx ~top_down ~bottom_up;
      }

    and (transform_ty_record :
          record ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          record) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_record with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_record elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_record with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_record elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_record elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_record elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_record with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_record elem ~ctx ~top_down ~bottom_up))

    and (traverse_ty_variant :
          variant ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          variant) =
     fun variant ~ctx ~top_down ~bottom_up ->
      match variant with
      | Normal normal_elem ->
        Normal (transform_ty_record normal_elem ~ctx ~top_down ~bottom_up)
      | Tuple_with_opaque
          ( tuple_with_opaque_elem_0,
            tuple_with_opaque_elem_1,
            tuple_with_opaque_elem_2 ) ->
        Tuple_with_opaque
          ( transform_ty_record
              tuple_with_opaque_elem_0
              ~ctx
              ~top_down
              ~bottom_up,
            tuple_with_opaque_elem_1,
            tuple_with_opaque_elem_2 )
      | variant -> variant

    and (transform_ty_variant :
          variant ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          variant) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_variant with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_variant elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_variant with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_variant elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_variant elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_variant elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_variant with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_variant elem ~ctx ~top_down ~bottom_up))

    let _ = traverse_ty_alias

    and _ = transform_ty_alias

    and _ = traverse_ty_record

    and _ = transform_ty_record

    and _ = traverse_ty_variant

    and _ = transform_ty_variant
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Explicit_annotations : sig
  type variant =
    | Constant
    | Inline_record of {
        a: int;
        b: record;
      }
    | Single of record [@transform.explicit]
    | Tuple of alias * record [@transform.explicit]

  and record = { variant: variant [@transform.explicit] }

  and alias = (record * variant[@transform.explicit]) [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_alias:
          (alias ->
          ctx:'ctx ->
          'ctx * [ `Stop of alias | `Continue of alias | `Restart of alias ])
          option;
        on_ty_record:
          (record ->
          ctx:'ctx ->
          'ctx * [ `Stop of record | `Continue of record | `Restart of record ])
          option;
        on_fld_record_variant:
          (variant ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of variant | `Continue of variant | `Restart of variant ])
          option;
        on_ty_variant:
          (variant ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of variant | `Continue of variant | `Restart of variant ])
          option;
        on_ctor_variant_Single:
          (record ->
          ctx:'ctx ->
          'ctx * [ `Stop of record | `Continue of record | `Restart of record ])
          option;
        on_ctor_variant_Tuple:
          (alias * record ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of alias * record
            | `Continue of alias * record
            | `Restart of alias * record
            ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform_ty_alias :
      alias ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      alias

    val transform_ty_record :
      record ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      record

    val transform_fld_record_variant :
      variant ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      variant

    val transform_ty_variant :
      variant ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      variant

    val transform_ctor_variant_Single :
      record ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      record

    val transform_ctor_variant_Tuple :
      alias * record ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      alias * record
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type variant =
    | Constant
    | Inline_record of {
        a: int;
        b: record;
      }
    | Single of record [@transform.explicit]
    | Tuple of alias * record [@transform.explicit]

  and record = { variant: variant [@transform.explicit] }

  and alias = (record * variant[@transform.explicit]) [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : variant) -> ())

    let _ = (fun (_ : record) -> ())

    let _ = (fun (_ : alias) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_alias:
          (alias ->
          ctx:'ctx ->
          'ctx * [ `Stop of alias | `Continue of alias | `Restart of alias ])
          option;
        on_ty_record:
          (record ->
          ctx:'ctx ->
          'ctx * [ `Stop of record | `Continue of record | `Restart of record ])
          option;
        on_fld_record_variant:
          (variant ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of variant | `Continue of variant | `Restart of variant ])
          option;
        on_ty_variant:
          (variant ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of variant | `Continue of variant | `Restart of variant ])
          option;
        on_ctor_variant_Single:
          (record ->
          ctx:'ctx ->
          'ctx * [ `Stop of record | `Continue of record | `Restart of record ])
          option;
        on_ctor_variant_Tuple:
          (alias * record ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of alias * record
            | `Continue of alias * record
            | `Restart of alias * record
            ])
          option;
      }

      let identity _ =
        {
          on_ty_alias = None;
          on_ty_record = None;
          on_fld_record_variant = None;
          on_ty_variant = None;
          on_ctor_variant_Single = None;
          on_ctor_variant_Tuple = None;
        }

      let _ = identity

      let combine p1 p2 =
        {
          on_ty_alias =
            (match (p1.on_ty_alias, p2.on_ty_alias) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_alias
            | _ -> p1.on_ty_alias);
          on_ty_record =
            (match (p1.on_ty_record, p2.on_ty_record) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_record
            | _ -> p1.on_ty_record);
          on_fld_record_variant =
            (match (p1.on_fld_record_variant, p2.on_fld_record_variant) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_fld_record_variant
            | _ -> p1.on_fld_record_variant);
          on_ty_variant =
            (match (p1.on_ty_variant, p2.on_ty_variant) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_variant
            | _ -> p1.on_ty_variant);
          on_ctor_variant_Single =
            (match (p1.on_ctor_variant_Single, p2.on_ctor_variant_Single) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ctor_variant_Single
            | _ -> p1.on_ctor_variant_Single);
          on_ctor_variant_Tuple =
            (match (p1.on_ctor_variant_Tuple, p2.on_ctor_variant_Tuple) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ctor_variant_Tuple
            | _ -> p1.on_ctor_variant_Tuple);
        }

      let _ = combine
    end

    let rec (traverse_ty_alias :
              alias ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              alias) =
     fun (alias_0, alias_1) ~ctx ~top_down ~bottom_up ->
      ( transform_ty_record alias_0 ~ctx ~top_down ~bottom_up,
        transform_ty_variant alias_1 ~ctx ~top_down ~bottom_up )

    and (transform_ty_alias :
          alias ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          alias) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_alias with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_alias elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_alias with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_alias elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_alias elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_alias elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_alias with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_alias elem ~ctx ~top_down ~bottom_up))

    and (traverse_ty_record :
          record ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          record) =
     fun { variant } ~ctx ~top_down ~bottom_up ->
      {
        variant = transform_fld_record_variant variant ~ctx ~top_down ~bottom_up;
      }

    and (transform_ty_record :
          record ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          record) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_record with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_record elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_record with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_record elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_record elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_record elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_record with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_record elem ~ctx ~top_down ~bottom_up))

    and (traverse_fld_record_variant :
          variant ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          variant) =
     fun record_variant ~ctx ~top_down ~bottom_up ->
      transform_ty_variant record_variant ~ctx ~top_down ~bottom_up

    and (transform_fld_record_variant :
          variant ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          variant) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_fld_record_variant with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_fld_record_variant elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_fld_record_variant with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_fld_record_variant elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_fld_record_variant elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_fld_record_variant elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_fld_record_variant with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_fld_record_variant elem ~ctx ~top_down ~bottom_up))

    and (traverse_ty_variant :
          variant ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          variant) =
     fun variant ~ctx ~top_down ~bottom_up ->
      match variant with
      | Inline_record ({ b; _ } as inline_record) ->
        Inline_record
          {
            inline_record with
            b = transform_ty_record b ~ctx ~top_down ~bottom_up;
          }
      | Single elem ->
        Single (transform_ctor_variant_Single elem ~ctx ~top_down ~bottom_up)
      | Tuple (elem_0, elem_1) ->
        let (elem_0, elem_1) =
          transform_ctor_variant_Tuple
            (elem_0, elem_1)
            ~ctx
            ~top_down
            ~bottom_up
        in
        Tuple (elem_0, elem_1)
      | variant -> variant

    and (transform_ty_variant :
          variant ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          variant) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_variant with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_variant elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_variant with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_variant elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_variant elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_variant elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_variant with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_variant elem ~ctx ~top_down ~bottom_up))

    and (traverse_ctor_variant_Single :
          record ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          record) =
     fun variant_Single ~ctx ~top_down ~bottom_up ->
      transform_ty_record variant_Single ~ctx ~top_down ~bottom_up

    and (transform_ctor_variant_Single :
          record ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          record) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ctor_variant_Single with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ctor_variant_Single elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ctor_variant_Single with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ctor_variant_Single elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ctor_variant_Single elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem =
          traverse_ctor_variant_Single elem ~ctx ~top_down ~bottom_up
        in
        (match bottom_up.Pass.on_ctor_variant_Single with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ctor_variant_Single elem ~ctx ~top_down ~bottom_up))

    and (traverse_ctor_variant_Tuple :
          alias * record ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          alias * record) =
     fun (variant_Tuple_0, variant_Tuple_1) ~ctx ~top_down ~bottom_up ->
      ( transform_ty_alias variant_Tuple_0 ~ctx ~top_down ~bottom_up,
        transform_ty_record variant_Tuple_1 ~ctx ~top_down ~bottom_up )

    and (transform_ctor_variant_Tuple :
          alias * record ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          alias * record) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ctor_variant_Tuple with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ctor_variant_Tuple elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ctor_variant_Tuple with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ctor_variant_Tuple elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ctor_variant_Tuple elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ctor_variant_Tuple elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ctor_variant_Tuple with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ctor_variant_Tuple elem ~ctx ~top_down ~bottom_up))

    let _ = traverse_ty_alias

    and _ = transform_ty_alias

    and _ = traverse_ty_record

    and _ = transform_ty_record

    and _ = traverse_fld_record_variant

    and _ = transform_fld_record_variant

    and _ = traverse_ty_variant

    and _ = transform_ty_variant

    and _ = traverse_ctor_variant_Single

    and _ = transform_ctor_variant_Single

    and _ = traverse_ctor_variant_Tuple

    and _ = transform_ctor_variant_Tuple
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Explicit_annotations_gadt : sig
  type z = Z : z

  and 'n s = S : 'n -> 'n s

  and ('a, _) gtree =
    | EmptyG : ('a, z) gtree
    | TreeG : ('a, 'n) gtree * 'a * ('a, 'n) gtree -> ('a, 'n s) gtree
        [@transform.explicit]
  [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_z:
          (z ->
          ctx:'ctx ->
          'ctx * [ `Stop of z | `Continue of z | `Restart of z ])
          option;
        on_ty_s:
          'a.
          ('a s ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a s | `Continue of 'a s | `Restart of 'a s ])
          option;
        on_ty_gtree:
          'a 'b.
          (('a, 'b) gtree ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of ('a, 'b) gtree
            | `Continue of ('a, 'b) gtree
            | `Restart of ('a, 'b) gtree
            ])
          option;
        on_ctor_gtree_TreeG:
          'a 'n.
          (('a, 'n) gtree * 'a * ('a, 'n) gtree ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of ('a, 'n) gtree * 'a * ('a, 'n) gtree
            | `Continue of ('a, 'n) gtree * 'a * ('a, 'n) gtree
            | `Restart of ('a, 'n) gtree * 'a * ('a, 'n) gtree
            ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform_ty_z :
      z -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> z

    val transform_ty_s :
      'a s -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> 'a s

    val transform_ty_gtree :
      ('a, 'b) gtree ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      ('a, 'b) gtree

    val transform_ctor_gtree_TreeG :
      ('a, 'n) gtree * 'a * ('a, 'n) gtree ->
      ctx:'ctx ->
      top_down:'ctx Pass.t ->
      bottom_up:'ctx Pass.t ->
      ('a, 'n) gtree * 'a * ('a, 'n) gtree
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type z = Z : z

  and 'n s = S : 'n -> 'n s

  and ('a, _) gtree =
    | EmptyG : ('a, z) gtree
    | TreeG : ('a, 'n) gtree * 'a * ('a, 'n) gtree -> ('a, 'n s) gtree
        [@transform.explicit]
  [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : z) -> ())

    let _ = (fun (_ : 'n s) -> ())

    let _ = (fun (_ : ('a, _) gtree) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_z:
          (z ->
          ctx:'ctx ->
          'ctx * [ `Stop of z | `Continue of z | `Restart of z ])
          option;
        on_ty_s:
          'a.
          ('a s ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a s | `Continue of 'a s | `Restart of 'a s ])
          option;
        on_ty_gtree:
          'a 'b.
          (('a, 'b) gtree ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of ('a, 'b) gtree
            | `Continue of ('a, 'b) gtree
            | `Restart of ('a, 'b) gtree
            ])
          option;
        on_ctor_gtree_TreeG:
          'a 'n.
          (('a, 'n) gtree * 'a * ('a, 'n) gtree ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of ('a, 'n) gtree * 'a * ('a, 'n) gtree
            | `Continue of ('a, 'n) gtree * 'a * ('a, 'n) gtree
            | `Restart of ('a, 'n) gtree * 'a * ('a, 'n) gtree
            ])
          option;
      }

      let identity _ =
        {
          on_ty_z = None;
          on_ty_s = None;
          on_ty_gtree = None;
          on_ctor_gtree_TreeG = None;
        }

      let _ = identity

      let combine p1 p2 =
        {
          on_ty_z =
            (match (p1.on_ty_z, p2.on_ty_z) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_z
            | _ -> p1.on_ty_z);
          on_ty_s =
            (match (p1.on_ty_s, p2.on_ty_s) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_s
            | _ -> p1.on_ty_s);
          on_ty_gtree =
            (match (p1.on_ty_gtree, p2.on_ty_gtree) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_gtree
            | _ -> p1.on_ty_gtree);
          on_ctor_gtree_TreeG =
            (match (p1.on_ctor_gtree_TreeG, p2.on_ctor_gtree_TreeG) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ctor_gtree_TreeG
            | _ -> p1.on_ctor_gtree_TreeG);
        }

      let _ = combine
    end

    let rec (transform_ty_z :
              z ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              z) =
     fun (elem : z) ~ctx ~top_down ~bottom_up : z ->
      match top_down.Pass.on_ty_z with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (_ctx, `Continue elem) ->
          (match bottom_up.Pass.on_ty_z with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_z elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) -> transform_ty_z elem ~ctx ~top_down ~bottom_up)
      | _ ->
        (match bottom_up.Pass.on_ty_z with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_z elem ~ctx ~top_down ~bottom_up))

    let _ = transform_ty_z

    let rec transform_ty_s :
              'a.
              'a s ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a s =
      fun (type a) (elem : a s) ~ctx ~top_down ~bottom_up : a s ->
       match top_down.Pass.on_ty_s with
       | Some td ->
         (match td elem ~ctx with
         | (_ctx, `Stop elem) -> elem
         | (_ctx, `Continue elem) ->
           (match bottom_up.Pass.on_ty_s with
           | None -> elem
           | Some bu ->
             (match bu elem ~ctx with
             | (_ctx, (`Continue elem | `Stop elem)) -> elem
             | (_ctx, `Restart elem) ->
               transform_ty_s elem ~ctx ~top_down ~bottom_up))
         | (_ctx, `Restart elem) ->
           transform_ty_s elem ~ctx ~top_down ~bottom_up)
       | _ ->
         (match bottom_up.Pass.on_ty_s with
         | None -> elem
         | Some bu ->
           (match bu elem ~ctx with
           | (_ctx, (`Continue elem | `Stop elem)) -> elem
           | (_ctx, `Restart elem) ->
             transform_ty_s elem ~ctx ~top_down ~bottom_up))

    let _ = transform_ty_s

    let rec traverse_ty_gtree :
              'a 'b.
              ('a, 'b) gtree ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              ('a, 'b) gtree =
      fun (type a b) (gtree : (a, b) gtree) ~ctx ~top_down ~bottom_up :
          (a, b) gtree ->
       match gtree with
       | TreeG (elem_0, elem_1, elem_2) ->
         let (elem_0, elem_1, elem_2) =
           transform_ctor_gtree_TreeG
             (elem_0, elem_1, elem_2)
             ~ctx
             ~top_down
             ~bottom_up
         in
         TreeG (elem_0, elem_1, elem_2)
       | gtree -> gtree

    and transform_ty_gtree :
          'a 'b.
          ('a, 'b) gtree ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          ('a, 'b) gtree =
      fun (type a b) (elem : (a, b) gtree) ~ctx ~top_down ~bottom_up :
          (a, b) gtree ->
       match top_down.Pass.on_ty_gtree with
       | Some td ->
         (match td elem ~ctx with
         | (_ctx, `Stop elem) -> elem
         | (td_ctx, `Continue elem) ->
           let elem = traverse_ty_gtree elem ~ctx:td_ctx ~top_down ~bottom_up in
           (match bottom_up.Pass.on_ty_gtree with
           | None -> elem
           | Some bu ->
             (match bu elem ~ctx with
             | (_ctx, (`Continue elem | `Stop elem)) -> elem
             | (_ctx, `Restart elem) ->
               transform_ty_gtree elem ~ctx ~top_down ~bottom_up))
         | (_ctx, `Restart elem) ->
           transform_ty_gtree elem ~ctx ~top_down ~bottom_up)
       | _ ->
         let elem = traverse_ty_gtree elem ~ctx ~top_down ~bottom_up in
         (match bottom_up.Pass.on_ty_gtree with
         | None -> elem
         | Some bu ->
           (match bu elem ~ctx with
           | (_ctx, (`Continue elem | `Stop elem)) -> elem
           | (_ctx, `Restart elem) ->
             transform_ty_gtree elem ~ctx ~top_down ~bottom_up))

    and traverse_ctor_gtree_TreeG :
          'a 'n.
          ('a, 'n) gtree * 'a * ('a, 'n) gtree ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          ('a, 'n) gtree * 'a * ('a, 'n) gtree =
     fun (gtree_TreeG_0, gtree_TreeG_1, gtree_TreeG_2) ~ctx ~top_down ~bottom_up ->
      ( transform_ty_gtree gtree_TreeG_0 ~ctx ~top_down ~bottom_up,
        gtree_TreeG_1,
        transform_ty_gtree gtree_TreeG_2 ~ctx ~top_down ~bottom_up )

    and transform_ctor_gtree_TreeG :
          'a 'n.
          ('a, 'n) gtree * 'a * ('a, 'n) gtree ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          ('a, 'n) gtree * 'a * ('a, 'n) gtree =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ctor_gtree_TreeG with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ctor_gtree_TreeG elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ctor_gtree_TreeG with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ctor_gtree_TreeG elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ctor_gtree_TreeG elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ctor_gtree_TreeG elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ctor_gtree_TreeG with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ctor_gtree_TreeG elem ~ctx ~top_down ~bottom_up))

    let _ = traverse_ty_gtree

    and _ = transform_ty_gtree

    and _ = traverse_ctor_gtree_TreeG

    and _ = transform_ctor_gtree_TreeG
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module No_restart : sig
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform ~restart:(`Disallow `Encode_as_variant)]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_t:
          (t -> ctx:'ctx -> 'ctx * [ `Stop of t | `Continue of t ]) option;
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
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform ~restart:(`Disallow `Encode_as_variant)]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : t) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_t:
          (t -> ctx:'ctx -> 'ctx * [ `Stop of t | `Continue of t ]) option;
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
      | Plus (plus_elem_0, plus_elem_1) ->
        Plus
          ( transform plus_elem_0 ~ctx ~top_down ~bottom_up,
            transform plus_elem_1 ~ctx ~top_down ~bottom_up )
      | Leq (leq_elem_0, leq_elem_1) ->
        Leq
          ( transform leq_elem_0 ~ctx ~top_down ~bottom_up,
            transform leq_elem_1 ~ctx ~top_down ~bottom_up )
      | Cond (cond_elem_0, cond_elem_1, cond_elem_2) ->
        Cond
          ( transform cond_elem_0 ~ctx ~top_down ~bottom_up,
            transform cond_elem_1 ~ctx ~top_down ~bottom_up,
            transform cond_elem_2 ~ctx ~top_down ~bottom_up )
      | t -> t

    and (transform :
          t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_t with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem)))
      | _ ->
        let elem = traverse elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_t with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem))

    let _ = traverse

    and _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module No_restart_as_result : sig
  type t =
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform ~restart:(`Disallow `Encode_as_result)]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_t: (t -> ctx:'ctx -> 'ctx * (t, t) result) option;
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
    | Num of int
    | Plus of t * t
    | Leq of t * t
    | Cond of t * t * t
  [@@deriving transform ~restart:(`Disallow `Encode_as_result)]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : t) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_t: (t -> ctx:'ctx -> 'ctx * (t, t) result) option;
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
                  | (ctx, Ok elem) -> t2 elem ~ctx
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
      | Plus (plus_elem_0, plus_elem_1) ->
        Plus
          ( transform plus_elem_0 ~ctx ~top_down ~bottom_up,
            transform plus_elem_1 ~ctx ~top_down ~bottom_up )
      | Leq (leq_elem_0, leq_elem_1) ->
        Leq
          ( transform leq_elem_0 ~ctx ~top_down ~bottom_up,
            transform leq_elem_1 ~ctx ~top_down ~bottom_up )
      | Cond (cond_elem_0, cond_elem_1, cond_elem_2) ->
        Cond
          ( transform cond_elem_0 ~ctx ~top_down ~bottom_up,
            transform cond_elem_1 ~ctx ~top_down ~bottom_up,
            transform cond_elem_2 ~ctx ~top_down ~bottom_up )
      | t -> t

    and (transform :
          t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_t with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, Error elem) -> elem
        | (td_ctx, Ok elem) ->
          let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (Ok elem | Error elem)) -> elem)))
      | _ ->
        let elem = traverse elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_t with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (Ok elem | Error elem)) -> elem))

    let _ = traverse

    and _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Variants = struct
  module Other : sig
    type t = { stuff: int } [@@deriving transform]

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
    type t = { stuff: int } [@@deriving transform]

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

      let rec (transform :
                t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                t) =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (_ctx, `Continue elem) ->
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module Exactly : sig
    type t =
      [ `A of Other.t
      | `B of Other.t
      ]
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
          on_Other: 'ctx Other.Pass.t option;
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
      [ `A of Other.t
      | `B of Other.t
      ]
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
          on_Other: 'ctx Other.Pass.t option;
        }

        let identity _ = { on_ty_t = None; on_Other = None }

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
            on_Other =
              (match (p1.on_Other, p2.on_Other) with
              | (Some p1, Some p2) -> Some (Other.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_Other
              | _ -> p2.on_Other);
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
        | `A t_elem ->
          `A
            (match (top_down.Pass.on_Other, bottom_up.Pass.on_Other) with
            | (Some top_down, Some bottom_up) ->
              Other.transform t_elem ~ctx ~top_down ~bottom_up
            | (Some top_down, _) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down
                ~bottom_up:(Other.Pass.identity ())
            | (_, Some bottom_up) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down:(Other.Pass.identity ())
                ~bottom_up
            | _ -> t_elem)
        | `B t_elem ->
          `B
            (match (top_down.Pass.on_Other, bottom_up.Pass.on_Other) with
            | (Some top_down, Some bottom_up) ->
              Other.transform t_elem ~ctx ~top_down ~bottom_up
            | (Some top_down, _) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down
                ~bottom_up:(Other.Pass.identity ())
            | (_, Some bottom_up) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down:(Other.Pass.identity ())
                ~bottom_up
            | _ -> t_elem)

      and (transform :
            t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t)
          =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (td_ctx, `Continue elem) ->
            let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          let elem = traverse elem ~ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module Extend : sig
    type t =
      [ `More of Other.t
      | `Zero
      | Exactly.t
      ]
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
          on_Exactly: 'ctx Exactly.Pass.t option;
          on_Other: 'ctx Other.Pass.t option;
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
      [ `More of Other.t
      | `Zero
      | Exactly.t
      ]
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
          on_Exactly: 'ctx Exactly.Pass.t option;
          on_Other: 'ctx Other.Pass.t option;
        }

        let identity _ = { on_ty_t = None; on_Exactly = None; on_Other = None }

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
            on_Exactly =
              (match (p1.on_Exactly, p2.on_Exactly) with
              | (Some p1, Some p2) -> Some (Exactly.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_Exactly
              | _ -> p2.on_Exactly);
            on_Other =
              (match (p1.on_Other, p2.on_Other) with
              | (Some p1, Some p2) -> Some (Other.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_Other
              | _ -> p2.on_Other);
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
        | `More t_elem ->
          `More
            (match (top_down.Pass.on_Other, bottom_up.Pass.on_Other) with
            | (Some top_down, Some bottom_up) ->
              Other.transform t_elem ~ctx ~top_down ~bottom_up
            | (Some top_down, _) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down
                ~bottom_up:(Other.Pass.identity ())
            | (_, Some bottom_up) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down:(Other.Pass.identity ())
                ~bottom_up
            | _ -> t_elem)
        | #Exactly.t as t_extend ->
          (match (top_down.Pass.on_Exactly, bottom_up.Pass.on_Exactly) with
           | (Some top_down, Some bottom_up) ->
             Exactly.transform t_extend ~ctx ~top_down ~bottom_up
           | (Some top_down, _) ->
             Exactly.transform
               t_extend
               ~ctx
               ~top_down
               ~bottom_up:(Exactly.Pass.identity ())
           | (_, Some bottom_up) ->
             Exactly.transform
               t_extend
               ~ctx
               ~top_down:(Exactly.Pass.identity ())
               ~bottom_up
           | _ -> t_extend
            :> [ `More of Other.t | `Zero | Exactly.t ])
        | t -> t

      and (transform :
            t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t)
          =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (td_ctx, `Continue elem) ->
            let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          let elem = traverse elem ~ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module At_least : sig
    type 'a t = [> `A of Other.t | `B of Other.t ] as 'a [@@deriving transform]

    include sig
      [@@@ocaml.warning "-32-60"]

      module Pass : sig
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
            option;
          on_Other: 'ctx Other.Pass.t option;
        }

        val combine : 'ctx t -> 'ctx t -> 'ctx t

        val identity : unit -> 'ctx t
      end

      val transform :
        'a t ->
        ctx:'ctx ->
        top_down:'ctx Pass.t ->
        bottom_up:'ctx Pass.t ->
        'a t
    end
    [@@ocaml.doc "@inline"] [@@merlin.hide]
  end = struct
    type 'a t = [> `A of Other.t | `B of Other.t ] as 'a [@@deriving transform]

    include struct
      [@@@ocaml.warning "-60"]

      let _ = (fun (_ : 'a t) -> ())

      module Pass = struct
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
            option;
          on_Other: 'ctx Other.Pass.t option;
        }

        let identity _ = { on_ty_t = None; on_Other = None }

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
            on_Other =
              (match (p1.on_Other, p2.on_Other) with
              | (Some p1, Some p2) -> Some (Other.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_Other
              | _ -> p2.on_Other);
          }

        let _ = combine
      end

      let rec traverse :
                'a.
                'a t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                'a t =
       fun t ~ctx ~top_down ~bottom_up ->
        match t with
        | `A t_elem ->
          `A
            (match (top_down.Pass.on_Other, bottom_up.Pass.on_Other) with
            | (Some top_down, Some bottom_up) ->
              Other.transform t_elem ~ctx ~top_down ~bottom_up
            | (Some top_down, _) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down
                ~bottom_up:(Other.Pass.identity ())
            | (_, Some bottom_up) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down:(Other.Pass.identity ())
                ~bottom_up
            | _ -> t_elem)
        | `B t_elem ->
          `B
            (match (top_down.Pass.on_Other, bottom_up.Pass.on_Other) with
            | (Some top_down, Some bottom_up) ->
              Other.transform t_elem ~ctx ~top_down ~bottom_up
            | (Some top_down, _) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down
                ~bottom_up:(Other.Pass.identity ())
            | (_, Some bottom_up) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down:(Other.Pass.identity ())
                ~bottom_up
            | _ -> t_elem)
        | t -> t

      and transform :
            'a.
            'a t ->
            ctx:'ctx ->
            top_down:'ctx Pass.t ->
            bottom_up:'ctx Pass.t ->
            'a t =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (td_ctx, `Continue elem) ->
            let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          let elem = traverse elem ~ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module At_most : sig
    type 'a t = [< `A of Other.t | `B of Other.t ] as 'a [@@deriving transform]

    include sig
      [@@@ocaml.warning "-32-60"]

      module Pass : sig
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
            option;
          on_Other: 'ctx Other.Pass.t option;
        }

        val combine : 'ctx t -> 'ctx t -> 'ctx t

        val identity : unit -> 'ctx t
      end

      val transform :
        'a t ->
        ctx:'ctx ->
        top_down:'ctx Pass.t ->
        bottom_up:'ctx Pass.t ->
        'a t
    end
    [@@ocaml.doc "@inline"] [@@merlin.hide]
  end = struct
    type 'a t = [< `A of Other.t | `B of Other.t ] as 'a [@@deriving transform]

    include struct
      [@@@ocaml.warning "-60"]

      let _ = (fun (_ : 'a t) -> ())

      module Pass = struct
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
            option;
          on_Other: 'ctx Other.Pass.t option;
        }

        let identity _ = { on_ty_t = None; on_Other = None }

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
            on_Other =
              (match (p1.on_Other, p2.on_Other) with
              | (Some p1, Some p2) -> Some (Other.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_Other
              | _ -> p2.on_Other);
          }

        let _ = combine
      end

      let rec transform :
                'a.
                'a t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                'a t =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (_ctx, `Continue elem) ->
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module As_most_and_at_least : sig
    type 'a t = [< `A of Other.t | `B of Other.t > `A ] as 'a
    [@@deriving transform]

    include sig
      [@@@ocaml.warning "-32-60"]

      module Pass : sig
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
            option;
          on_Other: 'ctx Other.Pass.t option;
        }

        val combine : 'ctx t -> 'ctx t -> 'ctx t

        val identity : unit -> 'ctx t
      end

      val transform :
        'a t ->
        ctx:'ctx ->
        top_down:'ctx Pass.t ->
        bottom_up:'ctx Pass.t ->
        'a t
    end
    [@@ocaml.doc "@inline"] [@@merlin.hide]
  end = struct
    type 'a t = [< `A of Other.t | `B of Other.t > `A ] as 'a
    [@@deriving transform]

    include struct
      [@@@ocaml.warning "-60"]

      let _ = (fun (_ : 'a t) -> ())

      module Pass = struct
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
            option;
          on_Other: 'ctx Other.Pass.t option;
        }

        let identity _ = { on_ty_t = None; on_Other = None }

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
            on_Other =
              (match (p1.on_Other, p2.on_Other) with
              | (Some p1, Some p2) -> Some (Other.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_Other
              | _ -> p2.on_Other);
          }

        let _ = combine
      end

      let rec traverse :
                'a.
                'a t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                'a t =
       fun t ~ctx ~top_down ~bottom_up ->
        match t with
        | `A t_elem ->
          `A
            (match (top_down.Pass.on_Other, bottom_up.Pass.on_Other) with
            | (Some top_down, Some bottom_up) ->
              Other.transform t_elem ~ctx ~top_down ~bottom_up
            | (Some top_down, _) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down
                ~bottom_up:(Other.Pass.identity ())
            | (_, Some bottom_up) ->
              Other.transform
                t_elem
                ~ctx
                ~top_down:(Other.Pass.identity ())
                ~bottom_up
            | _ -> t_elem)
        | t -> t

      and transform :
            'a.
            'a t ->
            ctx:'ctx ->
            top_down:'ctx Pass.t ->
            bottom_up:'ctx Pass.t ->
            'a t =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (td_ctx, `Continue elem) ->
            let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          let elem = traverse elem ~ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end
end

module Polymorphic = struct
  module Other : sig
    type 'a t = Self of 'a t option [@@deriving transform]

    include sig
      [@@@ocaml.warning "-32-60"]

      module Pass : sig
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
            option;
        }

        val combine : 'ctx t -> 'ctx t -> 'ctx t

        val identity : unit -> 'ctx t
      end

      val transform :
        'a t ->
        ctx:'ctx ->
        top_down:'ctx Pass.t ->
        bottom_up:'ctx Pass.t ->
        'a t
    end
    [@@ocaml.doc "@inline"] [@@merlin.hide]
  end = struct
    type 'a t = Self of 'a t option [@@deriving transform]

    include struct
      [@@@ocaml.warning "-60"]

      let _ = (fun (_ : 'a t) -> ())

      module Pass = struct
        type nonrec 'ctx t = {
          on_ty_t:
            'a.
            ('a t ->
            ctx:'ctx ->
            'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
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

      let rec traverse :
                'a.
                'a t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                'a t =
       fun t ~ctx ~top_down ~bottom_up ->
        match t with
        | Self self_elem ->
          Self
            (match self_elem with
            | Some self_elem_inner ->
              Some (transform self_elem_inner ~ctx ~top_down ~bottom_up)
            | _ -> None)

      and transform :
            'a.
            'a t ->
            ctx:'ctx ->
            top_down:'ctx Pass.t ->
            bottom_up:'ctx Pass.t ->
            'a t =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (td_ctx, `Continue elem) ->
            let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          let elem = traverse elem ~ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  module Quantified : sig
    type t = { forall: 'a. 'a Other.t } [@@deriving transform]

    include sig
      [@@@ocaml.warning "-32-60"]

      module Pass : sig
        type nonrec 'ctx t = {
          on_ty_t:
            (t ->
            ctx:'ctx ->
            'ctx * [ `Stop of t | `Continue of t | `Restart of t ])
            option;
          on_Other: 'ctx Other.Pass.t option;
        }

        val combine : 'ctx t -> 'ctx t -> 'ctx t

        val identity : unit -> 'ctx t
      end

      val transform :
        t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t
    end
    [@@ocaml.doc "@inline"] [@@merlin.hide]
  end = struct
    type t = { forall: 'a. 'a Other.t } [@@deriving transform]

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
          on_Other: 'ctx Other.Pass.t option;
        }

        let identity _ = { on_ty_t = None; on_Other = None }

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
            on_Other =
              (match (p1.on_Other, p2.on_Other) with
              | (Some p1, Some p2) -> Some (Other.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_Other
              | _ -> p2.on_Other);
          }

        let _ = combine
      end

      let rec (traverse :
                t ->
                ctx:'ctx ->
                top_down:'ctx Pass.t ->
                bottom_up:'ctx Pass.t ->
                t) =
       fun { forall } ~ctx ~top_down ~bottom_up ->
        {
          forall =
            (match (top_down.Pass.on_Other, bottom_up.Pass.on_Other) with
            | (Some top_down, Some bottom_up) ->
              Other.transform forall ~ctx ~top_down ~bottom_up
            | (Some top_down, _) ->
              Other.transform
                forall
                ~ctx
                ~top_down
                ~bottom_up:(Other.Pass.identity ())
            | (_, Some bottom_up) ->
              Other.transform
                forall
                ~ctx
                ~top_down:(Other.Pass.identity ())
                ~bottom_up
            | _ -> forall);
        }

      and (transform :
            t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t)
          =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (td_ctx, `Continue elem) ->
            let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          let elem = traverse elem ~ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end
end

module Recursive_mod = struct
  module rec One : sig
    type t = One of Two.t option [@@deriving transform]

    include sig
      [@@@ocaml.warning "-32-60"]

      module Pass : sig
        type nonrec 'ctx t = {
          on_ty_t:
            (t ->
            ctx:'ctx ->
            'ctx * [ `Stop of t | `Continue of t | `Restart of t ])
            option;
          on_Two: 'ctx Two.Pass.t option;
        }

        val combine : 'ctx t -> 'ctx t -> 'ctx t

        val identity : unit -> 'ctx t
      end

      val transform :
        t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t
    end
    [@@ocaml.doc "@inline"] [@@merlin.hide]
  end = struct
    type t = One of Two.t option [@@deriving transform]

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
          on_Two: 'ctx Two.Pass.t option;
        }

        let identity _ = { on_ty_t = None; on_Two = None }

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
            on_Two =
              (match (p1.on_Two, p2.on_Two) with
              | (Some p1, Some p2) -> Some (Two.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_Two
              | _ -> p2.on_Two);
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
        | One one_elem ->
          One
            (match one_elem with
            | Some one_elem_inner ->
              Some
                (match (top_down.Pass.on_Two, bottom_up.Pass.on_Two) with
                | (Some top_down, Some bottom_up) ->
                  Two.transform one_elem_inner ~ctx ~top_down ~bottom_up
                | (Some top_down, _) ->
                  Two.transform
                    one_elem_inner
                    ~ctx
                    ~top_down
                    ~bottom_up:(Two.Pass.identity ())
                | (_, Some bottom_up) ->
                  Two.transform
                    one_elem_inner
                    ~ctx
                    ~top_down:(Two.Pass.identity ())
                    ~bottom_up
                | _ -> one_elem_inner)
            | _ -> None)

      and (transform :
            t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t)
          =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (td_ctx, `Continue elem) ->
            let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          let elem = traverse elem ~ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end

  and Two : sig
    type t = Two of One.t option [@@deriving transform]

    include sig
      [@@@ocaml.warning "-32-60"]

      module Pass : sig
        type nonrec 'ctx t = {
          on_ty_t:
            (t ->
            ctx:'ctx ->
            'ctx * [ `Stop of t | `Continue of t | `Restart of t ])
            option;
          on_One: 'ctx One.Pass.t option;
        }

        val combine : 'ctx t -> 'ctx t -> 'ctx t

        val identity : unit -> 'ctx t
      end

      val transform :
        t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t
    end
    [@@ocaml.doc "@inline"] [@@merlin.hide]
  end = struct
    type t = Two of One.t option [@@deriving transform]

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
          on_One: 'ctx One.Pass.t option;
        }

        let identity _ = { on_ty_t = None; on_One = None }

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
            on_One =
              (match (p1.on_One, p2.on_One) with
              | (Some p1, Some p2) -> Some (One.Pass.combine p1 p2)
              | (Some _, _) -> p1.on_One
              | _ -> p2.on_One);
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
        | Two two_elem ->
          Two
            (match two_elem with
            | Some two_elem_inner ->
              Some
                (match (top_down.Pass.on_One, bottom_up.Pass.on_One) with
                | (Some top_down, Some bottom_up) ->
                  One.transform two_elem_inner ~ctx ~top_down ~bottom_up
                | (Some top_down, _) ->
                  One.transform
                    two_elem_inner
                    ~ctx
                    ~top_down
                    ~bottom_up:(One.Pass.identity ())
                | (_, Some bottom_up) ->
                  One.transform
                    two_elem_inner
                    ~ctx
                    ~top_down:(One.Pass.identity ())
                    ~bottom_up
                | _ -> two_elem_inner)
            | _ -> None)

      and (transform :
            t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t)
          =
       fun elem ~ctx ~top_down ~bottom_up ->
        match top_down.Pass.on_ty_t with
        | Some td ->
          (match td elem ~ctx with
          | (_ctx, `Stop elem) -> elem
          | (td_ctx, `Continue elem) ->
            let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
            (match bottom_up.Pass.on_ty_t with
            | None -> elem
            | Some bu ->
              (match bu elem ~ctx with
              | (_ctx, (`Continue elem | `Stop elem)) -> elem
              | (_ctx, `Restart elem) ->
                transform elem ~ctx ~top_down ~bottom_up))
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
        | _ ->
          let elem = traverse elem ~ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

      let _ = traverse

      and _ = transform
    end [@@ocaml.doc "@inline"] [@@merlin.hide]
  end
end

module Indexed_gadt : sig
  type one

  type 'a t = Other : one t -> 'a t [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform :
      'a t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> 'a t
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type one

  type 'a t = Other : one t -> 'a t [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : 'a t) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
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

    let rec traverse :
              'a.
              'a t ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a t =
      fun (type a) (t : a t) ~ctx ~top_down ~bottom_up : a t ->
       match t with
       | Other other_elem ->
         Other (transform other_elem ~ctx ~top_down ~bottom_up)

    and transform :
          'a.
          'a t ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a t =
      fun (type a) (elem : a t) ~ctx ~top_down ~bottom_up : a t ->
       match top_down.Pass.on_ty_t with
       | Some td ->
         (match td elem ~ctx with
         | (_ctx, `Stop elem) -> elem
         | (td_ctx, `Continue elem) ->
           let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
           (match bottom_up.Pass.on_ty_t with
           | None -> elem
           | Some bu ->
             (match bu elem ~ctx with
             | (_ctx, (`Continue elem | `Stop elem)) -> elem
             | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))
         | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
       | _ ->
         let elem = traverse elem ~ctx ~top_down ~bottom_up in
         (match bottom_up.Pass.on_ty_t with
         | None -> elem
         | Some bu ->
           (match bu elem ~ctx with
           | (_ctx, (`Continue elem | `Stop elem)) -> elem
           | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

    let _ = traverse

    and _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module Mixed_adt_gadt : sig
  type _ t =
    | Adt_data_ctor_nullary
    | Gadt_data_ctor_nullary : 'a t
    | Adt_data_ctor of int
    | Gadt_data_ctor : int -> string t
  [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform :
      'a t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> 'a t
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type _ t =
    | Adt_data_ctor_nullary
    | Gadt_data_ctor_nullary : 'a t
    | Adt_data_ctor of int
    | Gadt_data_ctor : int -> string t
  [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : _ t) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
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

    let rec transform :
              'a.
              'a t ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a t =
      fun (type a) (elem : a t) ~ctx ~top_down ~bottom_up : a t ->
       match top_down.Pass.on_ty_t with
       | Some td ->
         (match td elem ~ctx with
         | (_ctx, `Stop elem) -> elem
         | (_ctx, `Continue elem) ->
           (match bottom_up.Pass.on_ty_t with
           | None -> elem
           | Some bu ->
             (match bu elem ~ctx with
             | (_ctx, (`Continue elem | `Stop elem)) -> elem
             | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))
         | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
       | _ ->
         (match bottom_up.Pass.on_ty_t with
         | None -> elem
         | Some bu ->
           (match bu elem ~ctx with
           | (_ctx, (`Continue elem | `Stop elem)) -> elem
           | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

    let _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module MyList : sig
  type 'a t [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform :
      'a t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> 'a t
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type 'a t =
    | Empty
    | Cons of 'a * 'a t
  [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : 'a t) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
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

    let rec traverse :
              'a.
              'a t ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a t =
     fun t ~ctx ~top_down ~bottom_up ->
      match t with
      | Cons (cons_elem_0, cons_elem_1) ->
        Cons (cons_elem_0, transform cons_elem_1 ~ctx ~top_down ~bottom_up)
      | t -> t

    and transform :
          'a.
          'a t ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a t =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_t with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_t with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

    let _ = traverse

    and _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module MyAbstract : sig
  type 'a t [@@deriving transform]

  include sig
    [@@@ocaml.warning "-32-60"]

    module Pass : sig
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
          option;
      }

      val combine : 'ctx t -> 'ctx t -> 'ctx t

      val identity : unit -> 'ctx t
    end

    val transform :
      'a t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> 'a t
  end
  [@@ocaml.doc "@inline"] [@@merlin.hide]
end = struct
  type 'a t [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : 'a t) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_t:
          'a.
          ('a t ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a t | `Continue of 'a t | `Restart of 'a t ])
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

    let rec transform :
              'a.
              'a t ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a t =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_t with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (_ctx, `Continue elem) ->
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
      | _ ->
        (match bottom_up.Pass.on_ty_t with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

    let _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end

module SMap = Map.Make (String)
module TShapeMap = Map.Make (String)

module Hack_builtins : sig
  type t = { map: t SMap.t } [@@deriving transform]

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
  type t = { map: t SMap.t } [@@deriving transform]

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
     fun { map } ~ctx ~top_down ~bottom_up ->
      {
        map = SMap.map (fun map -> transform map ~ctx ~top_down ~bottom_up) map;
      }

    and (transform :
          t -> ctx:'ctx -> top_down:'ctx Pass.t -> bottom_up:'ctx Pass.t -> t) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_t with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_t with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_t with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) -> transform elem ~ctx ~top_down ~bottom_up))

    let _ = traverse

    and _ = transform
  end [@@ocaml.doc "@inline"] [@@merlin.hide]

  let depth_limit t ~n =
    let on_ty_t t ~ctx =
      if ctx >= n then
        (ctx, `Stop { map = SMap.empty })
      else
        (ctx + 1, `Continue t)
    in
    let bottom_up = Pass.identity ()
    and top_down =
      let open Pass in
      { on_ty_t = Some on_ty_t }
    in
    transform t ~ctx:0 ~top_down ~bottom_up
end

module Reason = struct
  type 'a t_
end

module TanySentinel = struct
  type t
end

module Tvid = struct
  type t
end

module Typing_defs_core = struct
  type decl_phase = private Decl_Phase

  type locl_phase = private Locl_Phase

  type tprim

  type type_predicate

  type type_origin

  type pos_id = string

  type variance

  type constraint_kind

  type reify_kind

  type user_attribute

  type dependent_type

  type 'ty tparam = {
    tp_tparams: 'ty tparam list;
    tp_constraints: (constraint_kind * 'ty) list;
  }

  let rec map_tparam f { tp_tparams; tp_constraints } =
    let tp_tparams = List.map (map_tparam f) tp_tparams
    and tp_constraints = List.map (fun (e0, e1) -> (e0, f e1)) tp_constraints in
    { tp_tparams; tp_constraints }

  type 'ty where_constraint = 'ty * constraint_kind * 'ty

  let map_where_constraint f (e0, e1, e2) = (f e0, e1, f e2)

  type enforcement =
    | Unenforced
    | Enforced

  type 'ty capability =
    | CapDefaults
    | CapTy of 'ty

  let map_capability f t =
    match t with
    | CapTy e0 -> CapTy (f e0)
    | CapDefaults -> CapDefaults

  type 'ty fun_implicit_params = { capability: 'ty capability }
  [@@ocaml.doc
    " Companion to fun_params type, intended to consolidate checking of\n * implicit params for functions. "]

  let map_fun_implicit_params f { capability } =
    let capability = map_capability f capability in
    { capability }

  type 'ty fun_param = { fp_type: 'ty }

  let map_fun_param f { fp_type } =
    let fp_type = f fp_type in
    { fp_type }

  type 'ty fun_params = 'ty fun_param list

  let map_fun_params f = List.map (map_fun_param f)

  type 'ty fun_type = {
    ft_tparams: 'ty tparam list;
    ft_where_constraints: 'ty where_constraint list;
    ft_params: 'ty fun_params;
    ft_implicit_params: 'ty fun_implicit_params;
    ft_ret: 'ty;
  }
  [@@ocaml.doc " The type of a function AND a method. "]

  let map_fun_type
      f
      {
        ft_tparams;
        ft_where_constraints;
        ft_params;
        ft_implicit_params;
        ft_ret;
      } =
    let ft_tparams = List.map (map_tparam f) ft_tparams
    and ft_where_constraints =
      List.map (map_where_constraint f) ft_where_constraints
    and ft_params = map_fun_params f ft_params
    and ft_implicit_params = map_fun_implicit_params f ft_implicit_params
    and ft_ret = f ft_ret in
    { ft_tparams; ft_where_constraints; ft_params; ft_implicit_params; ft_ret }

  type 'phase ty = ('phase Reason.t_[@transform.opaque]) * 'phase ty_

  and decl_ty = decl_phase ty

  and locl_ty = locl_phase ty

  and 'phase shape_field_type = { sft_ty: 'phase ty }

  and _ ty_ =
    | Tthis : decl_phase ty_
        [@ocaml.doc " The late static bound type of a class "]
    | Tapply : (pos_id[@transform.opaque]) * decl_ty list -> decl_phase ty_
    | Trefinement : decl_ty * decl_phase class_refinement -> decl_phase ty_
    | Tmixed : decl_phase ty_
    | Twildcard : decl_phase ty_
    | Tlike : decl_ty -> decl_phase ty_
    | Tany : (TanySentinel.t[@transform.opaque]) -> 'phase ty_
    | Tnonnull : 'phase ty_
    | Tdynamic : 'phase ty_
    | Toption : 'phase ty -> 'phase ty_
    | Tprim : (tprim[@transform.opaque]) -> 'phase ty_
    | Tfun : 'phase ty fun_type -> 'phase ty_
    | Ttuple : 'phase tuple_type -> 'phase ty_
    | Tshape : 'phase shape_type -> 'phase ty_
    | Tgeneric : string * 'phase ty list -> 'phase ty_
    | Tunion : 'phase ty list -> 'phase ty_
    | Tintersection : 'phase ty list -> 'phase ty_
    | Tvec_or_dict : 'phase ty * 'phase ty -> 'phase ty_
    | Taccess : 'phase taccess_type -> 'phase ty_
    | Tclass_ptr : 'phase ty -> 'phase ty_
    | Tvar : (Tvid.t[@transform.opaque]) -> locl_phase ty_
    | Tnewtype : string * locl_phase ty list * locl_phase ty -> locl_phase ty_
    | Tunapplied_alias : string -> locl_phase ty_
    | Tdependent :
        (dependent_type[@transform.opaque]) * locl_ty
        -> locl_phase ty_
    | Tclass :
        (pos_id[@transform.opaque]) * exact * locl_ty list
        -> locl_phase ty_
    | Tneg : (type_predicate[@transform.opaque]) -> locl_phase ty_
    | Tlabel : string -> locl_phase ty_

  and 'phase taccess_type = 'phase ty * (pos_id[@transform.opaque])

  and exact =
    | Exact
    | Nonexact of locl_phase class_refinement

  and 'phase class_refinement = { cr_consts: 'phase refined_const SMap.t }

  and 'phase refined_const = { rc_bound: 'phase refined_const_bound }

  and 'phase refined_const_bound =
    | TRexact : 'phase ty -> 'phase refined_const_bound
    | TRloose : 'phase refined_const_bounds -> 'phase refined_const_bound

  and 'phase refined_const_bounds = {
    tr_lower: 'phase ty list;
    tr_upper: 'phase ty list;
  }

  and 'phase shape_type = {
    s_unknown_value: 'phase ty;
    s_fields: 'phase shape_field_type TShapeMap.t;
  }

  and 'phase tuple_type = {
    t_required: 'phase ty list;
    t_extra: 'phase tuple_extra;
  }

  and 'phase tuple_extra =
    | Textra of {
        t_optional: 'phase ty list;
        t_variadic: 'phase ty;
      }
    | Tsplat of 'phase ty
  [@@deriving transform]

  include struct
    [@@@ocaml.warning "-60"]

    let _ = (fun (_ : 'phase ty) -> ())

    let _ = (fun (_ : decl_ty) -> ())

    let _ = (fun (_ : locl_ty) -> ())

    let _ = (fun (_ : 'phase shape_field_type) -> ())

    let _ = (fun (_ : _ ty_) -> ())

    let _ = (fun (_ : 'phase taccess_type) -> ())

    let _ = (fun (_ : exact) -> ())

    let _ = (fun (_ : 'phase class_refinement) -> ())

    let _ = (fun (_ : 'phase refined_const) -> ())

    let _ = (fun (_ : 'phase refined_const_bound) -> ())

    let _ = (fun (_ : 'phase refined_const_bounds) -> ())

    let _ = (fun (_ : 'phase shape_type) -> ())

    let _ = (fun (_ : 'phase tuple_type) -> ())

    let _ = (fun (_ : 'phase tuple_extra) -> ())

    module Pass = struct
      type nonrec 'ctx t = {
        on_ty_tuple_extra:
          'a.
          ('a tuple_extra ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a tuple_extra
            | `Continue of 'a tuple_extra
            | `Restart of 'a tuple_extra
            ])
          option;
        on_ty_tuple_type:
          'a.
          ('a tuple_type ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a tuple_type
            | `Continue of 'a tuple_type
            | `Restart of 'a tuple_type
            ])
          option;
        on_ty_shape_type:
          'a.
          ('a shape_type ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a shape_type
            | `Continue of 'a shape_type
            | `Restart of 'a shape_type
            ])
          option;
        on_ty_refined_const_bounds:
          'a.
          ('a refined_const_bounds ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a refined_const_bounds
            | `Continue of 'a refined_const_bounds
            | `Restart of 'a refined_const_bounds
            ])
          option;
        on_ty_refined_const_bound:
          'a.
          ('a refined_const_bound ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a refined_const_bound
            | `Continue of 'a refined_const_bound
            | `Restart of 'a refined_const_bound
            ])
          option;
        on_ty_refined_const:
          'a.
          ('a refined_const ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a refined_const
            | `Continue of 'a refined_const
            | `Restart of 'a refined_const
            ])
          option;
        on_ty_class_refinement:
          'a.
          ('a class_refinement ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a class_refinement
            | `Continue of 'a class_refinement
            | `Restart of 'a class_refinement
            ])
          option;
        on_ty_exact:
          (exact ->
          ctx:'ctx ->
          'ctx * [ `Stop of exact | `Continue of exact | `Restart of exact ])
          option;
        on_ty_taccess_type:
          'a.
          ('a taccess_type ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a taccess_type
            | `Continue of 'a taccess_type
            | `Restart of 'a taccess_type
            ])
          option;
        on_ty_ty_:
          'a.
          ('a ty_ ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a ty_ | `Continue of 'a ty_ | `Restart of 'a ty_ ])
          option;
        on_ty_shape_field_type:
          'a.
          ('a shape_field_type ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of 'a shape_field_type
            | `Continue of 'a shape_field_type
            | `Restart of 'a shape_field_type
            ])
          option;
        on_ty_locl_ty:
          (locl_ty ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of locl_ty | `Continue of locl_ty | `Restart of locl_ty ])
          option;
        on_ty_decl_ty:
          (decl_ty ->
          ctx:'ctx ->
          'ctx
          * [ `Stop of decl_ty | `Continue of decl_ty | `Restart of decl_ty ])
          option;
        on_ty_ty:
          'a.
          ('a ty ->
          ctx:'ctx ->
          'ctx * [ `Stop of 'a ty | `Continue of 'a ty | `Restart of 'a ty ])
          option;
      }

      let identity _ =
        {
          on_ty_tuple_extra = None;
          on_ty_tuple_type = None;
          on_ty_shape_type = None;
          on_ty_refined_const_bounds = None;
          on_ty_refined_const_bound = None;
          on_ty_refined_const = None;
          on_ty_class_refinement = None;
          on_ty_exact = None;
          on_ty_taccess_type = None;
          on_ty_ty_ = None;
          on_ty_shape_field_type = None;
          on_ty_locl_ty = None;
          on_ty_decl_ty = None;
          on_ty_ty = None;
        }

      let _ = identity

      let combine p1 p2 =
        {
          on_ty_tuple_extra =
            (match (p1.on_ty_tuple_extra, p2.on_ty_tuple_extra) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_tuple_extra
            | _ -> p1.on_ty_tuple_extra);
          on_ty_tuple_type =
            (match (p1.on_ty_tuple_type, p2.on_ty_tuple_type) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_tuple_type
            | _ -> p1.on_ty_tuple_type);
          on_ty_shape_type =
            (match (p1.on_ty_shape_type, p2.on_ty_shape_type) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_shape_type
            | _ -> p1.on_ty_shape_type);
          on_ty_refined_const_bounds =
            (match
               (p1.on_ty_refined_const_bounds, p2.on_ty_refined_const_bounds)
             with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_refined_const_bounds
            | _ -> p1.on_ty_refined_const_bounds);
          on_ty_refined_const_bound =
            (match
               (p1.on_ty_refined_const_bound, p2.on_ty_refined_const_bound)
             with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_refined_const_bound
            | _ -> p1.on_ty_refined_const_bound);
          on_ty_refined_const =
            (match (p1.on_ty_refined_const, p2.on_ty_refined_const) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_refined_const
            | _ -> p1.on_ty_refined_const);
          on_ty_class_refinement =
            (match (p1.on_ty_class_refinement, p2.on_ty_class_refinement) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_class_refinement
            | _ -> p1.on_ty_class_refinement);
          on_ty_exact =
            (match (p1.on_ty_exact, p2.on_ty_exact) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_exact
            | _ -> p1.on_ty_exact);
          on_ty_taccess_type =
            (match (p1.on_ty_taccess_type, p2.on_ty_taccess_type) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_taccess_type
            | _ -> p1.on_ty_taccess_type);
          on_ty_ty_ =
            (match (p1.on_ty_ty_, p2.on_ty_ty_) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_ty_
            | _ -> p1.on_ty_ty_);
          on_ty_shape_field_type =
            (match (p1.on_ty_shape_field_type, p2.on_ty_shape_field_type) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_shape_field_type
            | _ -> p1.on_ty_shape_field_type);
          on_ty_locl_ty =
            (match (p1.on_ty_locl_ty, p2.on_ty_locl_ty) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_locl_ty
            | _ -> p1.on_ty_locl_ty);
          on_ty_decl_ty =
            (match (p1.on_ty_decl_ty, p2.on_ty_decl_ty) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_decl_ty
            | _ -> p1.on_ty_decl_ty);
          on_ty_ty =
            (match (p1.on_ty_ty, p2.on_ty_ty) with
            | (Some t1, Some t2) ->
              Some
                (fun elem ~ctx ->
                  match t1 elem ~ctx with
                  | (ctx, `Continue elem) -> t2 elem ~ctx
                  | otherwise -> otherwise)
            | (None, _) -> p2.on_ty_ty
            | _ -> p1.on_ty_ty);
        }

      let _ = combine
    end

    let rec traverse_ty_tuple_extra :
              'a.
              'a tuple_extra ->
              ctx:'ctx ->
              top_down:'ctx Pass.t ->
              bottom_up:'ctx Pass.t ->
              'a tuple_extra =
     fun tuple_extra ~ctx ~top_down ~bottom_up ->
      match tuple_extra with
      | Textra { t_optional; t_variadic } ->
        Textra
          {
            t_optional =
              Stdlib.List.map
                (fun t_optional ->
                  transform_ty_ty t_optional ~ctx ~top_down ~bottom_up)
                t_optional;
            t_variadic = transform_ty_ty t_variadic ~ctx ~top_down ~bottom_up;
          }
      | Tsplat tsplat_elem ->
        Tsplat (transform_ty_ty tsplat_elem ~ctx ~top_down ~bottom_up)

    and transform_ty_tuple_extra :
          'a.
          'a tuple_extra ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a tuple_extra =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_tuple_extra with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_tuple_extra elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_tuple_extra with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_tuple_extra elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_tuple_extra elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_tuple_extra elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_tuple_extra with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_tuple_extra elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_tuple_type :
          'a.
          'a tuple_type ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a tuple_type =
     fun { t_required; t_extra } ~ctx ~top_down ~bottom_up ->
      {
        t_required =
          Stdlib.List.map
            (fun t_required ->
              transform_ty_ty t_required ~ctx ~top_down ~bottom_up)
            t_required;
        t_extra = transform_ty_tuple_extra t_extra ~ctx ~top_down ~bottom_up;
      }

    and transform_ty_tuple_type :
          'a.
          'a tuple_type ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a tuple_type =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_tuple_type with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_tuple_type elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_tuple_type with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_tuple_type elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_tuple_type elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_tuple_type elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_tuple_type with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_tuple_type elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_shape_type :
          'a.
          'a shape_type ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a shape_type =
     fun { s_unknown_value; s_fields } ~ctx ~top_down ~bottom_up ->
      {
        s_unknown_value =
          transform_ty_ty s_unknown_value ~ctx ~top_down ~bottom_up;
        s_fields =
          TShapeMap.map
            (fun s_fields ->
              transform_ty_shape_field_type s_fields ~ctx ~top_down ~bottom_up)
            s_fields;
      }

    and transform_ty_shape_type :
          'a.
          'a shape_type ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a shape_type =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_shape_type with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_shape_type elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_shape_type with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_shape_type elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_shape_type elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_shape_type elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_shape_type with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_shape_type elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_refined_const_bounds :
          'a.
          'a refined_const_bounds ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a refined_const_bounds =
     fun { tr_lower; tr_upper } ~ctx ~top_down ~bottom_up ->
      {
        tr_lower =
          Stdlib.List.map
            (fun tr_lower -> transform_ty_ty tr_lower ~ctx ~top_down ~bottom_up)
            tr_lower;
        tr_upper =
          Stdlib.List.map
            (fun tr_upper -> transform_ty_ty tr_upper ~ctx ~top_down ~bottom_up)
            tr_upper;
      }

    and transform_ty_refined_const_bounds :
          'a.
          'a refined_const_bounds ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a refined_const_bounds =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_refined_const_bounds with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_refined_const_bounds
              elem
              ~ctx:td_ctx
              ~top_down
              ~bottom_up
          in
          (match bottom_up.Pass.on_ty_refined_const_bounds with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_refined_const_bounds elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_refined_const_bounds elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem =
          traverse_ty_refined_const_bounds elem ~ctx ~top_down ~bottom_up
        in
        (match bottom_up.Pass.on_ty_refined_const_bounds with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_refined_const_bounds elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_refined_const_bound :
          'a.
          'a refined_const_bound ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a refined_const_bound =
      fun (type a)
          (refined_const_bound : a refined_const_bound)
          ~ctx
          ~top_down
          ~bottom_up : a refined_const_bound ->
       match refined_const_bound with
       | TRexact trexact_elem ->
         TRexact (transform_ty_ty trexact_elem ~ctx ~top_down ~bottom_up)
       | TRloose trloose_elem ->
         TRloose
           (transform_ty_refined_const_bounds
              trloose_elem
              ~ctx
              ~top_down
              ~bottom_up)

    and transform_ty_refined_const_bound :
          'a.
          'a refined_const_bound ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a refined_const_bound =
      fun (type a) (elem : a refined_const_bound) ~ctx ~top_down ~bottom_up :
          a refined_const_bound ->
       match top_down.Pass.on_ty_refined_const_bound with
       | Some td ->
         (match td elem ~ctx with
         | (_ctx, `Stop elem) -> elem
         | (td_ctx, `Continue elem) ->
           let elem =
             traverse_ty_refined_const_bound
               elem
               ~ctx:td_ctx
               ~top_down
               ~bottom_up
           in
           (match bottom_up.Pass.on_ty_refined_const_bound with
           | None -> elem
           | Some bu ->
             (match bu elem ~ctx with
             | (_ctx, (`Continue elem | `Stop elem)) -> elem
             | (_ctx, `Restart elem) ->
               transform_ty_refined_const_bound elem ~ctx ~top_down ~bottom_up))
         | (_ctx, `Restart elem) ->
           transform_ty_refined_const_bound elem ~ctx ~top_down ~bottom_up)
       | _ ->
         let elem =
           traverse_ty_refined_const_bound elem ~ctx ~top_down ~bottom_up
         in
         (match bottom_up.Pass.on_ty_refined_const_bound with
         | None -> elem
         | Some bu ->
           (match bu elem ~ctx with
           | (_ctx, (`Continue elem | `Stop elem)) -> elem
           | (_ctx, `Restart elem) ->
             transform_ty_refined_const_bound elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_refined_const :
          'a.
          'a refined_const ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a refined_const =
     fun { rc_bound } ~ctx ~top_down ~bottom_up ->
      {
        rc_bound =
          transform_ty_refined_const_bound rc_bound ~ctx ~top_down ~bottom_up;
      }

    and transform_ty_refined_const :
          'a.
          'a refined_const ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a refined_const =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_refined_const with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_refined_const elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_refined_const with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_refined_const elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_refined_const elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_refined_const elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_refined_const with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_refined_const elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_class_refinement :
          'a.
          'a class_refinement ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a class_refinement =
     fun { cr_consts } ~ctx ~top_down ~bottom_up ->
      {
        cr_consts =
          SMap.map
            (fun cr_consts ->
              transform_ty_refined_const cr_consts ~ctx ~top_down ~bottom_up)
            cr_consts;
      }

    and transform_ty_class_refinement :
          'a.
          'a class_refinement ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a class_refinement =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_class_refinement with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_class_refinement elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_class_refinement with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_class_refinement elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_class_refinement elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem =
          traverse_ty_class_refinement elem ~ctx ~top_down ~bottom_up
        in
        (match bottom_up.Pass.on_ty_class_refinement with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_class_refinement elem ~ctx ~top_down ~bottom_up))

    and (traverse_ty_exact :
          exact ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          exact) =
     fun exact ~ctx ~top_down ~bottom_up ->
      match exact with
      | Nonexact nonexact_elem ->
        Nonexact
          (transform_ty_class_refinement
             nonexact_elem
             ~ctx
             ~top_down
             ~bottom_up)
      | exact -> exact

    and (transform_ty_exact :
          exact ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          exact) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_exact with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_exact elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_exact with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_exact elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_exact elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_exact elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_exact with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_exact elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_taccess_type :
          'a.
          'a taccess_type ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a taccess_type =
     fun (taccess_type_0, taccess_type_1) ~ctx ~top_down ~bottom_up ->
      (transform_ty_ty taccess_type_0 ~ctx ~top_down ~bottom_up, taccess_type_1)

    and transform_ty_taccess_type :
          'a.
          'a taccess_type ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a taccess_type =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_taccess_type with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_taccess_type elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_taccess_type with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_taccess_type elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_taccess_type elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_taccess_type elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_taccess_type with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_taccess_type elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_ty_ :
          'a.
          'a ty_ ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a ty_ =
      fun (type a) (ty_ : a ty_) ~ctx ~top_down ~bottom_up : a ty_ ->
       match ty_ with
       | Tapply (tapply_elem_0, tapply_elem_1) ->
         Tapply
           ( tapply_elem_0,
             Stdlib.List.map
               (fun tapply_elem_1 ->
                 transform_ty_decl_ty tapply_elem_1 ~ctx ~top_down ~bottom_up)
               tapply_elem_1 )
       | Trefinement (trefinement_elem_0, trefinement_elem_1) ->
         Trefinement
           ( transform_ty_decl_ty trefinement_elem_0 ~ctx ~top_down ~bottom_up,
             transform_ty_class_refinement
               trefinement_elem_1
               ~ctx
               ~top_down
               ~bottom_up )
       | Tlike tlike_elem ->
         Tlike (transform_ty_decl_ty tlike_elem ~ctx ~top_down ~bottom_up)
       | Toption toption_elem ->
         Toption (transform_ty_ty toption_elem ~ctx ~top_down ~bottom_up)
       | Tfun tfun_elem ->
         Tfun
           (map_fun_type
              (fun tfun_elem ->
                transform_ty_ty tfun_elem ~ctx ~top_down ~bottom_up)
              tfun_elem)
       | Ttuple ttuple_elem ->
         Ttuple (transform_ty_tuple_type ttuple_elem ~ctx ~top_down ~bottom_up)
       | Tshape tshape_elem ->
         Tshape (transform_ty_shape_type tshape_elem ~ctx ~top_down ~bottom_up)
       | Tgeneric (tgeneric_elem_0, tgeneric_elem_1) ->
         Tgeneric
           ( tgeneric_elem_0,
             Stdlib.List.map
               (fun tgeneric_elem_1 ->
                 transform_ty_ty tgeneric_elem_1 ~ctx ~top_down ~bottom_up)
               tgeneric_elem_1 )
       | Tunion tunion_elem ->
         Tunion
           (Stdlib.List.map
              (fun tunion_elem ->
                transform_ty_ty tunion_elem ~ctx ~top_down ~bottom_up)
              tunion_elem)
       | Tintersection tintersection_elem ->
         Tintersection
           (Stdlib.List.map
              (fun tintersection_elem ->
                transform_ty_ty tintersection_elem ~ctx ~top_down ~bottom_up)
              tintersection_elem)
       | Tvec_or_dict (tvec_or_dict_elem_0, tvec_or_dict_elem_1) ->
         Tvec_or_dict
           ( transform_ty_ty tvec_or_dict_elem_0 ~ctx ~top_down ~bottom_up,
             transform_ty_ty tvec_or_dict_elem_1 ~ctx ~top_down ~bottom_up )
       | Taccess taccess_elem ->
         Taccess
           (transform_ty_taccess_type taccess_elem ~ctx ~top_down ~bottom_up)
       | Tclass_ptr tclass_ptr_elem ->
         Tclass_ptr (transform_ty_ty tclass_ptr_elem ~ctx ~top_down ~bottom_up)
       | Tnewtype (tnewtype_elem_0, tnewtype_elem_1, tnewtype_elem_2) ->
         Tnewtype
           ( tnewtype_elem_0,
             Stdlib.List.map
               (fun tnewtype_elem_1 ->
                 transform_ty_ty tnewtype_elem_1 ~ctx ~top_down ~bottom_up)
               tnewtype_elem_1,
             transform_ty_ty tnewtype_elem_2 ~ctx ~top_down ~bottom_up )
       | Tdependent (tdependent_elem_0, tdependent_elem_1) ->
         Tdependent
           ( tdependent_elem_0,
             transform_ty_locl_ty tdependent_elem_1 ~ctx ~top_down ~bottom_up )
       | Tclass (tclass_elem_0, tclass_elem_1, tclass_elem_2) ->
         Tclass
           ( tclass_elem_0,
             transform_ty_exact tclass_elem_1 ~ctx ~top_down ~bottom_up,
             Stdlib.List.map
               (fun tclass_elem_2 ->
                 transform_ty_locl_ty tclass_elem_2 ~ctx ~top_down ~bottom_up)
               tclass_elem_2 )
       | ty_ -> ty_

    and transform_ty_ty_ :
          'a.
          'a ty_ ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a ty_ =
      fun (type a) (elem : a ty_) ~ctx ~top_down ~bottom_up : a ty_ ->
       match top_down.Pass.on_ty_ty_ with
       | Some td ->
         (match td elem ~ctx with
         | (_ctx, `Stop elem) -> elem
         | (td_ctx, `Continue elem) ->
           let elem = traverse_ty_ty_ elem ~ctx:td_ctx ~top_down ~bottom_up in
           (match bottom_up.Pass.on_ty_ty_ with
           | None -> elem
           | Some bu ->
             (match bu elem ~ctx with
             | (_ctx, (`Continue elem | `Stop elem)) -> elem
             | (_ctx, `Restart elem) ->
               transform_ty_ty_ elem ~ctx ~top_down ~bottom_up))
         | (_ctx, `Restart elem) ->
           transform_ty_ty_ elem ~ctx ~top_down ~bottom_up)
       | _ ->
         let elem = traverse_ty_ty_ elem ~ctx ~top_down ~bottom_up in
         (match bottom_up.Pass.on_ty_ty_ with
         | None -> elem
         | Some bu ->
           (match bu elem ~ctx with
           | (_ctx, (`Continue elem | `Stop elem)) -> elem
           | (_ctx, `Restart elem) ->
             transform_ty_ty_ elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_shape_field_type :
          'a.
          'a shape_field_type ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a shape_field_type =
     fun { sft_ty } ~ctx ~top_down ~bottom_up ->
      { sft_ty = transform_ty_ty sft_ty ~ctx ~top_down ~bottom_up }

    and transform_ty_shape_field_type :
          'a.
          'a shape_field_type ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a shape_field_type =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_shape_field_type with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_shape_field_type elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_shape_field_type with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_shape_field_type elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_shape_field_type elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem =
          traverse_ty_shape_field_type elem ~ctx ~top_down ~bottom_up
        in
        (match bottom_up.Pass.on_ty_shape_field_type with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_shape_field_type elem ~ctx ~top_down ~bottom_up))

    and (traverse_ty_locl_ty :
          locl_ty ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          locl_ty) =
     fun locl_ty ~ctx ~top_down ~bottom_up ->
      transform_ty_ty locl_ty ~ctx ~top_down ~bottom_up

    and (transform_ty_locl_ty :
          locl_ty ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          locl_ty) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_locl_ty with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_locl_ty elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_locl_ty with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_locl_ty elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_locl_ty elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_locl_ty elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_locl_ty with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_locl_ty elem ~ctx ~top_down ~bottom_up))

    and (traverse_ty_decl_ty :
          decl_ty ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          decl_ty) =
     fun decl_ty ~ctx ~top_down ~bottom_up ->
      transform_ty_ty decl_ty ~ctx ~top_down ~bottom_up

    and (transform_ty_decl_ty :
          decl_ty ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          decl_ty) =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_decl_ty with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem =
            traverse_ty_decl_ty elem ~ctx:td_ctx ~top_down ~bottom_up
          in
          (match bottom_up.Pass.on_ty_decl_ty with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_decl_ty elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_decl_ty elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_decl_ty elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_decl_ty with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_decl_ty elem ~ctx ~top_down ~bottom_up))

    and traverse_ty_ty :
          'a.
          'a ty ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a ty =
     fun (ty_0, ty_1) ~ctx ~top_down ~bottom_up ->
      (ty_0, transform_ty_ty_ ty_1 ~ctx ~top_down ~bottom_up)

    and transform_ty_ty :
          'a.
          'a ty ->
          ctx:'ctx ->
          top_down:'ctx Pass.t ->
          bottom_up:'ctx Pass.t ->
          'a ty =
     fun elem ~ctx ~top_down ~bottom_up ->
      match top_down.Pass.on_ty_ty with
      | Some td ->
        (match td elem ~ctx with
        | (_ctx, `Stop elem) -> elem
        | (td_ctx, `Continue elem) ->
          let elem = traverse_ty_ty elem ~ctx:td_ctx ~top_down ~bottom_up in
          (match bottom_up.Pass.on_ty_ty with
          | None -> elem
          | Some bu ->
            (match bu elem ~ctx with
            | (_ctx, (`Continue elem | `Stop elem)) -> elem
            | (_ctx, `Restart elem) ->
              transform_ty_ty elem ~ctx ~top_down ~bottom_up))
        | (_ctx, `Restart elem) ->
          transform_ty_ty elem ~ctx ~top_down ~bottom_up)
      | _ ->
        let elem = traverse_ty_ty elem ~ctx ~top_down ~bottom_up in
        (match bottom_up.Pass.on_ty_ty with
        | None -> elem
        | Some bu ->
          (match bu elem ~ctx with
          | (_ctx, (`Continue elem | `Stop elem)) -> elem
          | (_ctx, `Restart elem) ->
            transform_ty_ty elem ~ctx ~top_down ~bottom_up))

    let _ = traverse_ty_tuple_extra

    and _ = transform_ty_tuple_extra

    and _ = traverse_ty_tuple_type

    and _ = transform_ty_tuple_type

    and _ = traverse_ty_shape_type

    and _ = transform_ty_shape_type

    and _ = traverse_ty_refined_const_bounds

    and _ = transform_ty_refined_const_bounds

    and _ = traverse_ty_refined_const_bound

    and _ = transform_ty_refined_const_bound

    and _ = traverse_ty_refined_const

    and _ = transform_ty_refined_const

    and _ = traverse_ty_class_refinement

    and _ = transform_ty_class_refinement

    and _ = traverse_ty_exact

    and _ = transform_ty_exact

    and _ = traverse_ty_taccess_type

    and _ = transform_ty_taccess_type

    and _ = traverse_ty_ty_

    and _ = transform_ty_ty_

    and _ = traverse_ty_shape_field_type

    and _ = transform_ty_shape_field_type

    and _ = traverse_ty_locl_ty

    and _ = transform_ty_locl_ty

    and _ = traverse_ty_decl_ty

    and _ = transform_ty_decl_ty

    and _ = traverse_ty_ty

    and _ = transform_ty_ty
  end [@@ocaml.doc "@inline"] [@@merlin.hide]
end
