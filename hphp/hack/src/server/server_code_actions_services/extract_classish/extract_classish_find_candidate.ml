open Hh_prelude
module T = Extract_classish_types

let class_kind_supports_extraction =
  Ast_defs.(
    function
    | Cclass (Concrete | Abstract) -> true
    | Cinterface
    | Ctrait
    | Cenum
    | Cenum_class _ ->
      false)

let find_candidate ~(selection : Pos.t) ~entry ctx : T.candidate option =
  let { Tast_provider.Compute_tast.tast; _ } =
    Tast_provider.compute_tast_quarantined ~ctx ~entry
  in

  List.find_map
    tast
    ~f:
      Aast_defs.(
        function
        | Class class_
          when Pos.contains class_.c_span selection
               && class_kind_supports_extraction class_.c_kind ->
          let selected_methods =
            class_.c_methods
            |> List.filter ~f:(fun meth -> Pos.contains selection meth.m_span)
          in
          if List.is_empty selected_methods then
            None
          else
            Some T.{ class_; selected_methods }
        | _ -> None)
