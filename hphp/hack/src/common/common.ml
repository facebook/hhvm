module List = struct
  include Core.List

  let unzip4 xyzws =
    let rec aux ((xs, ys, zs, ws) as acc) = function
      | (x, y, z, w) :: rest -> aux (x :: xs, y :: ys, z :: zs, w :: ws) rest
      | _ -> acc
    in
    aux ([], [], [], []) (List.rev xyzws)

  let rec fold_left_env env l ~init ~f =
    match l with
    | [] -> (env, init)
    | x :: xs ->
      let (env, init) = f env init x in
      fold_left_env env xs ~init ~f

  let rec fold_left_env_res env l ~init ~err ~f =
    match l with
    | [] -> (env, init, err)
    | x :: xs ->
      let (env, init, err) = f env init err x in
      fold_left_env_res env xs ~init ~err ~f

  let rev_map_env env xs ~f =
    let f2 env init x =
      let (env, x) = f env x in
      (env, x :: init)
    in
    fold_left_env env xs ~init:[] ~f:f2

  let rev_map_env_ty_err_opt ?(err = []) env xs ~f =
    let f2 env init errs x =
      match f env x with
      | ((env, Some err), x) -> (env, x :: init, err :: errs)
      | ((env, _), x) -> (env, x :: init, errs)
    in
    let (env, xs, errs) = fold_left_env_res env xs ~init:[] ~err ~f:f2 in
    ((env, errs), xs)

  let rev_map_env_res env xs ~f =
    let f2 env init errs x =
      let (env, x, err) = f env x in
      (env, x :: init, err :: errs)
    in
    fold_left_env_res env xs ~init:[] ~err:[] ~f:f2

  let map_env env xs ~f =
    let rec aux env xs counter =
      match xs with
      | [] -> (env, [])
      | [y1] ->
        let (env, z1) = f env y1 in
        (env, [z1])
      | [y1; y2] ->
        let (env, z1) = f env y1 in
        let (env, z2) = f env y2 in
        (env, [z1; z2])
      | [y1; y2; y3] ->
        let (env, z1) = f env y1 in
        let (env, z2) = f env y2 in
        let (env, z3) = f env y3 in
        (env, [z1; z2; z3])
      | [y1; y2; y3; y4] ->
        let (env, z1) = f env y1 in
        let (env, z2) = f env y2 in
        let (env, z3) = f env y3 in
        let (env, z4) = f env y4 in
        (env, [z1; z2; z3; z4])
      | [y1; y2; y3; y4; y5] ->
        let (env, z1) = f env y1 in
        let (env, z2) = f env y2 in
        let (env, z3) = f env y3 in
        let (env, z4) = f env y4 in
        let (env, z5) = f env y5 in
        (env, [z1; z2; z3; z4; z5])
      | y1 :: y2 :: y3 :: y4 :: y5 :: ys ->
        let (env, z1) = f env y1 in
        let (env, z2) = f env y2 in
        let (env, z3) = f env y3 in
        let (env, z4) = f env y4 in
        let (env, z5) = f env y5 in
        let (env, zs) =
          if counter > 1000 then
            let (env, zs) = rev_map_env env ys ~f in
            (env, rev zs)
          else
            aux env ys (counter + 1)
        in
        (env, z1 :: z2 :: z3 :: z4 :: z5 :: zs)
    in
    aux env xs 0

  let map_env_ty_err_opt
      env xs ~(f : 'a -> 'b -> ('a * 'e option) * 'c) ~combine_ty_errs =
    let rec aux env xs counter =
      match xs with
      | [] -> ((env, []), [])
      | [y1] ->
        let ((env, ty_err_opt), z1) = f env y1 in
        ((env, Option.to_list ty_err_opt), [z1])
      | [y1; y2] ->
        let ((env, ty_err_opt1), z1) = f env y1 in
        let ((env, ty_err_opt2), z2) = f env y2 in
        let ty_errs = List.filter_map (fun x -> x) [ty_err_opt1; ty_err_opt2] in
        ((env, ty_errs), [z1; z2])
      | [y1; y2; y3] ->
        let ((env, ty_err_opt1), z1) = f env y1 in
        let ((env, ty_err_opt2), z2) = f env y2 in
        let ((env, ty_err_opt3), z3) = f env y3 in
        let ty_errs =
          List.filter_map (fun x -> x) [ty_err_opt1; ty_err_opt2; ty_err_opt3]
        in
        ((env, ty_errs), [z1; z2; z3])
      | [y1; y2; y3; y4] ->
        let ((env, ty_err_opt1), z1) = f env y1 in
        let ((env, ty_err_opt2), z2) = f env y2 in
        let ((env, ty_err_opt3), z3) = f env y3 in
        let ((env, ty_err_opt4), z4) = f env y4 in
        let ty_errs =
          List.filter_map
            (fun x -> x)
            [ty_err_opt1; ty_err_opt2; ty_err_opt3; ty_err_opt4]
        in
        ((env, ty_errs), [z1; z2; z3; z4])
      | [y1; y2; y3; y4; y5] ->
        let ((env, ty_err_opt1), z1) = f env y1 in
        let ((env, ty_err_opt2), z2) = f env y2 in
        let ((env, ty_err_opt3), z3) = f env y3 in
        let ((env, ty_err_opt4), z4) = f env y4 in
        let ((env, ty_err_opt5), z5) = f env y5 in
        let ty_errs =
          List.filter_map
            (fun x -> x)
            [ty_err_opt1; ty_err_opt2; ty_err_opt3; ty_err_opt4; ty_err_opt5]
        in
        ((env, ty_errs), [z1; z2; z3; z4; z5])
      | y1 :: y2 :: y3 :: y4 :: y5 :: ys ->
        let ((env, ty_err_opt1), z1) = f env y1 in
        let ((env, ty_err_opt2), z2) = f env y2 in
        let ((env, ty_err_opt3), z3) = f env y3 in
        let ((env, ty_err_opt4), z4) = f env y4 in
        let ((env, ty_err_opt5), z5) = f env y5 in
        let err =
          List.filter_map
            (fun x -> x)
            [ty_err_opt1; ty_err_opt2; ty_err_opt3; ty_err_opt4; ty_err_opt5]
        in
        let (env, zs) =
          if counter > 1000 then
            let (env, zs) = rev_map_env_ty_err_opt env ys ~f ~err in
            (env, rev zs)
          else
            aux env ys (counter + 1)
        in
        (env, z1 :: z2 :: z3 :: z4 :: z5 :: zs)
    in
    let ((env, ty_errs), xs) = aux env xs 0 in
    ((env, combine_ty_errs ty_errs), xs)

  let map_env_err_res env xs ~f =
    let rec aux env xs counter =
      match xs with
      | [] -> (env, [], [])
      | [y1] ->
        let (env, z1, res) = f env y1 in
        (env, [z1], [res])
      | [y1; y2] ->
        let (env, z1, res1) = f env y1 in
        let (env, z2, res2) = f env y2 in
        (env, [z1; z2], [res1; res2])
      | [y1; y2; y3] ->
        let (env, z1, res1) = f env y1 in
        let (env, z2, res2) = f env y2 in
        let (env, z3, res3) = f env y3 in
        (env, [z1; z2; z3], [res1; res2; res3])
      | [y1; y2; y3; y4] ->
        let (env, z1, res1) = f env y1 in
        let (env, z2, res2) = f env y2 in
        let (env, z3, res3) = f env y3 in
        let (env, z4, res4) = f env y4 in
        (env, [z1; z2; z3; z4], [res1; res2; res3; res4])
      | [y1; y2; y3; y4; y5] ->
        let (env, z1, res1) = f env y1 in
        let (env, z2, res2) = f env y2 in
        let (env, z3, res3) = f env y3 in
        let (env, z4, res4) = f env y4 in
        let (env, z5, res5) = f env y5 in
        (env, [z1; z2; z3; z4; z5], [res1; res2; res3; res4; res5])
      | y1 :: y2 :: y3 :: y4 :: y5 :: ys ->
        let (env, z1, res1) = f env y1 in
        let (env, z2, res2) = f env y2 in
        let (env, z3, res3) = f env y3 in
        let (env, z4, res4) = f env y4 in
        let (env, z5, res5) = f env y5 in
        let (env, zs, res6) =
          if counter > 1000 then
            let (env, zs, errs) = rev_map_env_res env ys ~f in
            (env, rev zs, rev errs)
          else
            aux env ys (counter + 1)
        in
        ( env,
          z1 :: z2 :: z3 :: z4 :: z5 :: zs,
          res1 :: res2 :: res3 :: res4 :: res5 :: res6 )
    in
    aux env xs 0

  let rec map2_env env l1 l2 ~f =
    match (l1, l2) with
    | ([], []) -> (env, [])
    | ([], _)
    | (_, []) ->
      raise @@ Invalid_argument "map2_env"
    | (x1 :: rl1, x2 :: rl2) ->
      let (env, x) = f env x1 x2 in
      let (env, rl) = map2_env env rl1 rl2 ~f in
      (env, x :: rl)

  let map2_env_ty_err_opt env l1 l2 ~f ~combine_ty_errs =
    let rec aux env l1 l2 =
      match (l1, l2) with
      | ([], []) -> (env, [], [])
      | ([], _)
      | (_, []) ->
        raise @@ Invalid_argument "map2_env_ty_err_opt"
      | (x1 :: rl1, x2 :: rl2) ->
        let ((env, e), x) = f env x1 x2 in
        let (env, es, rl) = aux env rl1 rl2 in
        (env, e :: es, x :: rl)
    in
    let (env, errs, xs) = aux env l1 l2 in
    ((env, combine_ty_errs errs), xs)

  let rec map3_env env l1 l2 l3 ~f =
    if length l1 <> length l2 || length l2 <> length l3 then
      raise @@ Invalid_argument "map3_env"
    else
      match (l1, l2, l3) with
      | ([], [], []) -> (env, [])
      | ([], _, _)
      | (_, [], _)
      | (_, _, []) ->
        raise @@ Invalid_argument "map3_env"
      | (x1 :: rl1, x2 :: rl2, x3 :: rl3) ->
        let (env, x) = f env x1 x2 x3 in
        let (env, rl) = map3_env env rl1 rl2 rl3 ~f in
        (env, x :: rl)

  let filter_map_env env xs ~f =
    let (env, l) = rev_map_env env xs ~f in
    (env, rev_filter_map l ~f:(fun x -> x))

  let rec exists_env env xs ~f =
    match xs with
    | [] -> (env, false)
    | x :: xs ->
      (match f env x with
      | (env, true) -> (env, true)
      | (env, false) -> exists_env env xs ~f)

  let rec for_all_env env xs ~f =
    match xs with
    | [] -> (env, true)
    | x :: xs ->
      (match f env x with
      | (env, false) -> (env, false)
      | (env, true) -> for_all_env env xs ~f)

  let rec replicate ~num x =
    match num with
    | 0 -> []
    | n when n < 0 ->
      raise
      @@ Invalid_argument
           (Printf.sprintf "List.replicate was called with %d argument" n)
    | _ -> x :: replicate ~num:(num - 1) x
end

module Result = struct
  include Core.Result

  let fold t ~ok ~error =
    match t with
    | Ok x -> ok x
    | Error err -> error err
end
