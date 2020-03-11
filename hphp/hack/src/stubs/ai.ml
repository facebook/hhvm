(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let magic_builtins = [||]

let do_ _ _ _ = ()

let go _ _ _ _ _ = Errors.empty

let go_incremental _ _ _ _ _ = Errors.empty

let modify_shared_mem_sizes global_size heap_size dep_table_pow hash_table_pow _
    =
  (global_size, heap_size, dep_table_pow, hash_table_pow)

let modify_worker_count worker_count = worker_count

module InfoService = struct
  type target_type =
    | Function
    | Method
    | Constructor

  type fun_call = {
    name: string;
    type_: target_type;
    pos: Pos.absolute;
    caller: string;
    callees: string list; (* includes overrides, etc. *)
  }

  type throws = {
    thrower: string;
    (* the name of a function or method that throws/leaks *)
    filename: string;
    (* location of the function or method *)
    exceptions: string list; (* names of types of thrown exceptions *)
  }

  type result = {
    fun_calls: fun_call list;
    throws: throws list;
  }

  let empty_result = { fun_calls = []; throws = [] }

  let go _ _ _ _ _ = empty_result
end

module ServerFindDepFiles = struct
  let go _ _ _ = []
end

module ServerFindRefs = struct
  type member =
    | Method of string
    | Property of string
    | Class_const of string
    | Typeconst of string

  type action =
    | Class of string
    | Member of string * member
    | Function of string
    | GConst of string
    | LocalVar of {
        filename: Relative_path.t;
        file_content: string;
        line: int;
        char: int;
      }

  let go _ _ _ = []
end

module TraceService = struct
  type member =
    | Method of string
    | Property of string
    | Class_const of string
    | Typeconst of string

  type action =
    | Class of string
    | Member of string * member
    | Function of string
    | GConst of string
    | LocalVar of {
        filename: Relative_path.t;
        file_content: string;
        line: int;
        char: int;
      }

  let go _ _ _ _ = ""
end

module QueryService = struct
  let go _ _ = ""
end
