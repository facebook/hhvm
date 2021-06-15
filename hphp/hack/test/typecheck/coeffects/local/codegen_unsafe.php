<?hh

function io_disallowed_in_body()[codegen_unsafe]: void {
  echo "bad (missing `IO` capability)"; // error
  impure_stuff_such_as_io(); // ok
}

function impure_stuff_such_as_io(): void {
  echo "ok";
}

function sample_usage()[codegen]: void {
  io_disallowed_in_body(); // ok
}
