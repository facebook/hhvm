(* TODO(T266467978): stub — real implementation in next commit *)

let shape ~assign_pos:_ ~pat_pos env _ds _rhs_ty =
  let (env, ty) = Typing_env.fresh_type_error env pat_pos in
  (env, Tast.make_typed_expr pat_pos ty Aast.Omitted, ty)

let tuple ~assign_pos:_ ~pat_pos env _dt _rhs_ty =
  let (env, ty) = Typing_env.fresh_type_error env pat_pos in
  (env, Tast.make_typed_expr pat_pos ty Aast.Omitted, ty)
