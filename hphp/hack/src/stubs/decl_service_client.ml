type t = unit

let connect ~decl_state_dir:_ = failwith "not implemented"

let rpc_get_fun _ _ = failwith "not implemented"

let rpc_get_class _ _ = failwith "not implemneted"

let rpc_get_typedef _ _ = failwith "not implemented"

let rpc_get_gconst _ _ = failwith "not implemented"

let rpc_get_module _ _ = failwith "not implemented"

let rpc_get_folded_class _ _ = failwith "not implemented"

let rpc_store_folded_class _ _ _ = failwith "not implemented"

let rpc_get_type_kind _ _ = failwith "not implemented"

module Positioned = struct
  let rpc_get_typedef_path _ _ = failwith "not implemented"

  let rpc_get_full_pos _ _ _ _ = failwith "not implemented"
end

module Slow = struct
  let rpc_get_gconst_path _ _ = failwith "not implemented"

  let rpc_get_fun_path _ _ = failwith "not implemented"

  let rpc_get_type_path _ _ = failwith "not implemented"

  let rpc_get_module_path _ _ = failwith "not implemented"

  let rpc_get_fun_canon_name _ _ = failwith "not implemented"

  let rpc_get_type_canon_name _ _ = failwith "not implemented"
end
