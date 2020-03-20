open Core_kernel
open Typing_defs

exception Non_denotable

let print_tprim =
  Aast.(
    function
    | Tbool -> "bool"
    | Tint -> "int"
    | Tfloat -> "float"
    | Tnum -> "num"
    | Tstring -> "string"
    | Tarraykey -> "arraykey"
    | Tnull -> "null"
    | Tvoid -> "void"
    | Tresource -> "resource"
    | Tnoreturn -> "noreturn"
    | Tatom s -> ":@" ^ s)

let strip_ns str =
  let str' = Utils.strip_ns str in
  (* If we had more than one '\\' patternm then we must keep the first '\\'
  otherwise it will cause an error (we are detecting types from the root
  namespace)*)
  if String.contains str' '\\' then
    str
  else
    str'

let rec print_ty_exn ?(allow_nothing = false) ty =
  match get_node ty with
  | Tprim p -> print_tprim p
  | Tunion [] when allow_nothing -> "nothing"
  | Tdependent (DTthis, _) -> "this"
  | Tany _
  | Terr
  | Tvar _
  | Tdependent _
  | Tunion _
  | Tintersection _
  | Tobject ->
    raise Non_denotable
  | Tnonnull -> "nonnull"
  | Tdynamic -> "dynamic"
  | Tgeneric s -> s
  | Toption ty ->
    begin
      match get_node ty with
      | Tnonnull -> "mixed"
      | _ -> "?" ^ print_ty_exn ty
    end
  | Tfun ft ->
    let params = List.map ft.ft_params ~f:print_fun_param_exn in
    let params =
      match ft.ft_arity with
      | Fstandard _ -> params
      | Fellipsis _ -> raise Non_denotable
      | Fvariadic (_, p) -> params @ [print_ty_exn p.fp_type.et_type ^ "..."]
    in
    Printf.sprintf
      "(function(%s): %s)"
      (String.concat ~sep:", " params)
      (print_ty_exn ft.ft_ret.et_type)
  | Ttuple tyl -> "(" ^ print_tyl_exn tyl ^ ")"
  | Tshape (shape_kind, fdm) ->
    let fields =
      List.map (Nast.ShapeMap.elements fdm) ~f:print_shape_field_exn
    in
    let fields =
      match shape_kind with
      | Closed_shape -> fields
      | Open_shape -> fields @ ["..."]
    in
    Printf.sprintf "shape(%s)" (String.concat ~sep:", " fields)
  | Tnewtype (name, [], _) -> Utils.strip_ns name
  | Tnewtype (name, tyl, _) ->
    Utils.strip_ns name ^ "<" ^ print_tyl_exn tyl ^ ">"
  | Tclass ((_, name), _, []) -> strip_ns name
  | Tclass ((_, name), _, tyl) ->
    Utils.strip_ns name ^ "<" ^ print_tyl_exn tyl ^ ">"
  | Tarraykind (AKvarray ty) -> Printf.sprintf "varray<%s>" (print_ty_exn ty)
  | Tarraykind (AKvarray_or_darray (ty1, ty2)) ->
    Printf.sprintf
      "varray_or_darray<%s, %s>"
      (print_ty_exn ty1)
      (print_ty_exn ty2)
  | Tarraykind (AKdarray (ty1, ty2)) ->
    Printf.sprintf "darray<%s, %s>" (print_ty_exn ty1) (print_ty_exn ty2)
  | Tpu (ty, id) -> Printf.sprintf "(%s:@%s)" (print_ty_exn ty) (snd id)
  | Tpu_type_access (ty, id, member, tyname) ->
    Printf.sprintf
      "(%s:@%s:@%s:@%s)"
      (print_ty_exn ty)
      (snd id)
      (snd member)
      (snd tyname)

and print_tyl_exn tyl = String.concat ~sep:", " (List.map tyl ~f:print_ty_exn)

and print_fun_param_exn param =
  match param.fp_kind with
  | FPinout -> "inout " ^ print_ty_exn param.fp_type.et_type
  | _ -> print_ty_exn param.fp_type.et_type

and print_shape_field_exn (name, { sft_optional; sft_ty; _ }) =
  Printf.sprintf
    "%s%s => %s"
    ( if sft_optional then
      "?"
    else
      "" )
    (print_shape_field_name name)
    (print_ty_exn sft_ty)

and print_shape_field_name name =
  let s = Typing_env.get_shape_field_name name in
  match name with
  | Ast_defs.SFlit_str _ -> "'" ^ s ^ "'"
  | _ -> s

let print ?(allow_nothing = false) ty =
  (try Some (print_ty_exn ~allow_nothing ty) with Non_denotable -> None)
