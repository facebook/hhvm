(* Assert that the early counter starts at 1 and grows monotonically *)

let rec loop prev = function
  | 0 -> ()
  | n ->
    let i = Ident.tmp () in
    assert (prev < i);
    loop i (n - 1)

let () =
  let one = Ident.tmp () in
  assert (one == 1);
  loop one 10_000
