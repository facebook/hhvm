(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Hhbc_ast
open Instruction_sequence

(* The hhas assembler format doesn't currently support member keys of the
 * form EL:<local> using symbolic names. So we map local/parameter names
 * to their position number in a separate pass.
 *)
let unname_local_id env lid =
  match lid with
  | Local.Named name ->
    begin match SMap.get name env with
    | None -> lid
    | Some i -> Local.Named (string_of_int i)
    end
  | _ -> lid

let unname_member_key env mk =
  match mk with
  | MemberKey.EL lid -> MemberKey.EL (unname_local_id env lid)
  | MemberKey.PL lid -> MemberKey.PL (unname_local_id env lid)
  | _ -> mk

let unname_instr env instr =
  let u = unname_member_key env in
  match instr with
  | IBase (Dim(m, mk)) -> IBase (Dim(m, u mk))
  | IBase (FPassDim(i, mk)) -> IBase (FPassDim(i, u mk))
  | IFinal (QueryM(n, o, mk)) -> IFinal (QueryM(n, o, u mk))
  | IFinal (VGetM(n, mk)) -> IFinal (VGetM(n, u mk))
  | IFinal (FPassM(i, n, mk)) -> IFinal (FPassM(i, n, u mk))
  | IFinal (SetM(n, mk)) -> IFinal (SetM(n, u mk))
  | IFinal (IncDecM(n, o, mk)) -> IFinal (IncDecM(n, o, u mk))
  | IFinal (SetOpM(n, o, mk)) -> IFinal (SetOpM(n, o, u mk))
  | IFinal (BindM(n, mk)) -> IFinal (BindM(n, u mk))
  | IFinal (UnsetM(n, mk)) -> IFinal (UnsetM(n, u mk))
  | _ -> instr

let unname_instrseq vars instrs =
  let lid_env = List.foldi vars
      ~init:SMap.empty
      ~f:(fun i env name -> SMap.add name i env) in
  InstrSeq.map instrs (unname_instr lid_env)
