(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : ServerCommandTypes.Go_to_definition.result =
  let results =
    ServerIdentifyFunction.go_quarantined ~ctx ~entry ~line ~column
  in
  let results = List.filter_map results ~f:Utils.unwrap_snd in
  (* What's it like when we return multiple definitions? For instance, if you ask
   * for the definition of "new C()" then we've now got the definition of the
   * class "\C" and also of the constructor "\\C::__construct". I think that
   * users would be happier to only have the definition of the constructor, so
   * as to jump straight to it without the fuss of clicking to select which one.
   * That indeed is what Typescript does -- it only gives the constructor.
   * (VSCode displays multiple definitions with a peek view of them all;
   * Atom displays them with a small popup showing just title+file+line of each).
   * There's one subtlety. If you declare a base class "B" with a constructor,
   * and a derived class "C" without a constructor, and click on "new C()", then
   * Typescript and VS Code will pop up a little window with both options. This
   * seems like a reasonable compromise, so Hack should do the same. *)
  let cls =
    List.fold results ~init:`None ~f:(fun class_opt (occ, _) ->
        match (class_opt, SymbolOccurrence.enclosing_class occ) with
        | (`None, Some c) -> `Single c
        | (`Single c, Some c2) when String.equal c c2 -> `Single c
        | (`Single _, Some _) ->
          (* Symbol occurrences for methods/properties that only exist in a base
             class still have the derived class as their enclosing class, even
             though it doesn't explicitly override that member. Because of that, if
             we hit this case then we know that we're dealing with a union type. In
             that case, it's not really possible to do the rest of this filtration,
             since it would have to be decided on a per-class basis. *)
          `Multiple
        | (class_opt, _) -> class_opt)
  in
  let results =
    match cls with
    | `None
    | `Multiple ->
      results
    | `Single _ ->
      SymbolOccurrence.(
        let explicitly_defined =
          List.fold results ~init:[] ~f:(fun acc (occ, def) ->
              let cls = get_class_name occ in
              match cls with
              | None -> acc
              | Some cls ->
                if
                  String.is_prefix
                    def.SymbolDefinition.full_name
                    ~prefix:(Utils.strip_ns cls)
                then
                  (occ, def) :: acc
                else
                  acc)
          |> List.rev
        in
        let is_result_constructor (occ, _) = is_constructor occ in
        let has_explicit_constructor =
          List.exists explicitly_defined ~f:is_result_constructor
        in
        let has_constructor = List.exists results ~f:is_result_constructor in
        let has_class = List.exists results ~f:(fun (occ, _) -> is_class occ) in
        (* If we have a constructor but it's derived, then we'd like to show both
           the class and the constructor. If the constructor is explicitly
           defined, though, we'd like to filter the class out and only show the
           constructor. *)
        if has_constructor && has_class && has_explicit_constructor then
          List.filter results ~f:is_result_constructor
        else
          results)
  in
  List.map results ~f:(fun (occurrence, definition) ->
      let occurrence = SymbolOccurrence.to_absolute occurrence in
      let definition = SymbolDefinition.to_absolute definition in
      (occurrence, definition))
