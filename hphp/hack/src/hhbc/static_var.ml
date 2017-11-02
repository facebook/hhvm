open Hh_core

module A = Ast

class static_var_visitor = object
  inherit [A.expr option SMap.t] Ast_visitor.ast_visitor

  method! on_static_var acc el =
    List.fold_left el ~init:acc ~f: (fun acc e ->
      match snd e with
      | A.Lvar (_, name) -> SMap.add name None acc
      | A.Binop (A.Eq _, (_, A.Lvar (_, name)), exp) -> SMap.add name (Some exp) acc
      | _ -> failwith "Static var - impossible")

  method! on_class_ acc _ = acc
  method! on_fun_ acc _ = acc

end

let make_static_map b =
  let visitor = new static_var_visitor in
  visitor#on_program (SMap.empty) b
