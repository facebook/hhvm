type counter

external counter_new : unit -> counter = "counter_new"

external counter_inc : counter -> unit = "counter_inc"

external counter_read : counter -> int = "counter_read"

let () =
  let counter = counter_new () in
  assert (counter_read counter == 0);
  counter_inc counter;
  assert (counter_read counter == 1)
