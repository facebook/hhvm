open Core_kernel
open Typing_defs

let go env position =
  let (file, line, char) = position in
  let ServerEnv.{ tcopt; naming_table; _ } = env in
  let _, tast = ServerIdeUtils.check_file_input tcopt naming_table file in
  let env_and_ty = ServerInferType.type_at_pos tast line char in
  match env_and_ty with
  | None -> []
  | Some (env, (reason, ty)) ->
    let rec handle_type acc ty =
      match ty with
      | Tclass ((_, str), _, _) ->
        begin match NamingGlobal.GEnv.type_pos str with
        | None -> acc
        | Some pos -> (pos,str)::acc end
      | Toption (_, ty') ->
        handle_type acc ty'
      | Tunion ty_lst ->
        List.fold ty_lst ~init:acc ~f:(fun a (_,y) -> handle_type a y)
      | Tfun fn_type ->
        let (_, ret_type) = fn_type.ft_ret in begin
          match ret_type with
          | Tprim _ -> (* default to function definition *)
            (fn_type.ft_pos, Tast_env.print_ty env (reason, ty))::acc
          | _ -> handle_type acc ret_type
        end
      | _ -> acc
    in
    List.map (handle_type [] ty) ~f:(fun (pos, s) -> (Pos.to_absolute pos, s))
