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
