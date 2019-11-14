(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module Config = Random_ast_generator_config

module type AstGenerator = sig
  (* generate a program from the root non-terminal with given number of tokens*)
  val generate_with_exact_count : int -> Config.t -> string * int
end

module Make (G : Hack_grammar_descriptor_helper.Grammar) : AstGenerator = struct
  module NontermCount = struct
    type t = string * int

    let compare = compare
  end

  module PostfixCount = struct
    (* key is non-terminal, index of rule, offset position in rule, length *)
    type t = string * int * int * int

    let compare = compare
  end

  module CountMem = Map.Make (NontermCount)
  module PostfixMem = Map.Make (PostfixCount)

  type 'a memory = {
    count_mem: 'a CountMem.t;
    post_mem: 'a PostfixMem.t;
  }

  (*******************Preprocessing phase***********************)
  let sum (mem, lst) = (mem, List.fold_left ( + ) 0 lst)

  let to_list (mem, value) = (mem, [value])

  let get_symbol rule index = List.nth rule index

  let is_nonterm symbol =
    match symbol with
    | G.NonTerm _ -> true
    | _ -> false

  let normalizing_factor = 10000

  (* normalize the magnitude of the element in the list with the largest number
   * becoming 100 *)
  let normalize lst =
    let find_large acc el =
      if el > acc then
        el
      else
        acc
    in
    let max = List.fold_left find_large 0 lst in
    if max = 0 then
      lst
    else
      List.map (fun x -> x * normalizing_factor / max) lst

  let normalize_wrapper (mem, lst) = (mem, normalize lst)

  (* generate list from start to finish, excluding end *)
  let gen_list start finish =
    let rec aux start finish acc =
      if start > finish then
        acc
      else
        aux start (finish - 1) (finish :: acc)
    in
    aux start (finish - 1) []

  let put_count key value mem =
    let new_count_mem = CountMem.add key value mem.count_mem in
    { mem with count_mem = new_count_mem }

  let put_postfix key value mem =
    let new_post_mem = PostfixMem.add key value mem.post_mem in
    { mem with post_mem = new_post_mem }

  (* get the result of f and memoize the result *)
  let handle_count_mem mem key f =
    let (nonterm, count) = key in
    let name = G.nonterm_to_string nonterm in
    let new_key = (name, count) in
    if CountMem.mem new_key mem.count_mem then
      (mem, CountMem.find new_key mem.count_mem)
    else
      let (mem, ret) = f mem key in
      (put_count new_key ret mem, ret)

  (* get the result of f and memoize the result *)
  let handle_postfix_mem mem key f =
    let (nonterm, index, pos, count) = key in
    let new_key = (G.nonterm_to_string nonterm, index, pos, count) in
    if PostfixMem.mem new_key mem.post_mem then
      (mem, PostfixMem.find new_key mem.post_mem)
    else
      let (mem, ret) = f mem key in
      (put_postfix new_key ret mem, ret)

  (* Given nonterminal and expected string length, return a list of values,
   * for element for each rule starting with nonterm, with element valued at
   * the number of strings that can be produced of this string length *)
  let rec get_count_list mem nonterm count =
    handle_count_mem mem (nonterm, count) get_count_list_helper

  (* Given a nonterminal, a rule of production from it, the index of the
   * RHS symbol in the rule, and the target token number, this function
   * returns the number of strings that can be generated for each length of
   * the first symbol starting at the given index *)
  and get_postfix_count_list mem nonterm rule_index pos count =
    handle_postfix_mem mem (nonterm, rule_index, pos, count) get_postfix_helper

  and get_count_list_helper mem (nonterm, count) =
    let rules = G.grammar nonterm in
    let fold_fun (i, mem, lst) _ =
      let (mem, ret) = sum (get_postfix_count_list mem nonterm i 0 count) in
      (i + 1, mem, ret :: lst)
    in
    let (_, mem, lst) = List.fold_left fold_fun (0, mem, []) rules in
    (mem, lst |> normalize |> List.rev |> normalize)

  and get_postfix_helper mem (nonterm, rule_index, pos, count) =
    if count = 0 then
      (mem, [])
    else
      let rule = List.nth (G.grammar nonterm) rule_index in
      let first_symbol = get_symbol rule pos in
      let last_index = List.length rule - 1 in
      let after_first_symbol mem n_count =
        let n_pos = pos + 1 in
        sum (get_postfix_count_list mem nonterm rule_index n_pos n_count)
      in
      match first_symbol with
      | G.Term _ when pos = last_index ->
        ( mem,
          if count = 1 then
            [normalizing_factor]
          else
            [0] )
      | G.Term _ ->
        after_first_symbol mem (count - 1) |> to_list |> normalize_wrapper
      | G.NonTerm x when pos = last_index ->
        get_count_list mem x count |> sum |> to_list |> normalize_wrapper
      | G.NonTerm x ->
        (* all possible lengths the first symbol can expand into *)
        let all_lengths = gen_list 1 (count - last_index + pos + 1) in
        let fold_fun (mem, lst) l =
          let (mem, first_term_sum) = sum (get_count_list mem x l) in
          let (mem, later_terms_sum) = after_first_symbol mem (count - l) in
          let ret = first_term_sum * later_terms_sum in
          (mem, ret :: lst)
        in
        let (mem, lst) = List.fold_left fold_fun (mem, []) all_lengths in
        (mem, lst |> normalize |> List.rev)

  (************************** generation phase *************************)
  (* given [l1, l2, l3...] return index using probability according to weight *)
  let choose l =
    match l with
    | [] -> None
    | _ ->
      let folder (acc, lst) el =
        let total = acc + el in
        (total, total :: lst)
      in
      let (sum, cumulative) = List.fold_left folder (0, []) l in
      if sum = 0 then
        None
      else
        let (sum, cumulative) = (sum, List.rev cumulative) in
        let chosen = Random.int sum in
        let cumulative_with_index = List.mapi (fun i x -> (i, x)) cumulative in
        let predicate (_, el) = chosen < el in
        (try Some (fst (List.find predicate cumulative_with_index))
         with Not_found -> failwith "wrong implementation of choose")

  (* given a nonterminal and a target count, generate a string of that length
   * generated by this nonterminal, uniformly randomly over all possible string
   * of this length generated by the given non-terminal *)
  let rec get_string mem nonterm count =
    let (mem, lst) = get_count_list mem nonterm count in
    match choose lst with
    | None -> ""
    | Some r -> get_string_postfix mem nonterm r 0 count

  (* Generate a string of length [count] over all strings derivable from the
   * postfix of the production rule indexed at [rule_index] from [nonterm] *)
  and get_string_postfix mem nonterm rule_index pos count =
    let rule = List.nth (G.grammar nonterm) rule_index in
    let first_symbol = get_symbol rule pos in
    let last_index = List.length rule - 1 in
    let n_pos = pos + 1 in
    let n_count = count - 1 in
    match first_symbol with
    | G.Term x when last_index = pos -> G.to_string x
    | G.Term x ->
      let c_string = G.to_string x in
      let n_string = get_string_postfix mem nonterm rule_index n_pos n_count in
      String.concat " " [c_string; n_string]
    | G.NonTerm x when last_index = pos -> get_string mem x count
    | G.NonTerm x ->
      let (mem, lst) =
        get_postfix_count_list mem nonterm rule_index pos count
      in
      (match choose lst with
      | None -> ""
      | Some length ->
        let length = length + 1 in
        (* index to number of tokens *)
        let f_string = get_string mem x length in
        let rest_length = count - length in
        let e_string =
          get_string_postfix mem nonterm rule_index n_pos rest_length
        in
        String.concat " " [f_string; e_string])

  let gen_from_distribution distribution =
    let sum = List.fold_left ( +. ) 0. distribution in
    let target = Random.float sum in
    let fold_fun (acc, selected, count) el =
      if acc -. el <= 0. && selected < 0 then
        (acc -. el, count, count + 1)
      else
        (acc -. el, selected, count + 1)
    in
    let (_, choice, _) =
      List.fold_left fold_fun (target, -1, -1) distribution
    in
    choice

  let gen_from_distribution_opt config nonterm_name num_rules =
    if Config.exists nonterm_name config then
      let distribution = Config.get nonterm_name config in
      gen_from_distribution distribution
    else
      Random.int num_rules

  (* Stochastically generate strig: each production randomly chosen from
   * all productions of the current LHS. When a target count is reached, use
   * the dynamic programming generator to generate a short "postfix" *)
  let rec get_string_simple mem symbol count config =
    let short_len = 1 in
    match symbol with
    | G.Term x -> (mem, G.to_string x, 1)
    | G.NonTerm x ->
      if count <= 0 then
        let rec try_fun mem x len =
          let str = get_string mem x len in
          if str = "" then
            try_fun mem x (len + 1)
          else
            (str, len)
        in
        let (str, len) = try_fun mem x short_len in
        (mem, str, len)
      else
        let rules = G.grammar x in
        let num_rules = List.length rules in
        let nonterm_name = G.nonterm_to_string x in
        let index = gen_from_distribution_opt config nonterm_name num_rules in
        let rule = List.nth rules index in
        let nonterm_count =
          let count acc x =
            if is_nonterm x then
              acc + 1
            else
              acc
          in
          List.fold_left count 0 rule
        in
        let fold_fun (mem, str_lst, c) sym =
          let indi_count =
            if is_nonterm sym then
              count / nonterm_count
            else
              1
          in
          let (mem, str, el_c) = get_string_simple mem sym indi_count config in
          (mem, str :: str_lst, c + el_c)
        in
        let (mem, strings, generated) =
          List.fold_left fold_fun (mem, [], 0) rule
        in
        (mem, String.concat " " (List.rev strings), generated)

  let generate_all count config =
    let memory = { count_mem = CountMem.empty; post_mem = PostfixMem.empty } in
    get_string_simple memory (G.NonTerm G.start) count config

  let generate_with_exact_count count config =
    let (_, program, real_count) = generate_all count config in
    (Printf.sprintf "<?hh\n%s" program, real_count)
end
