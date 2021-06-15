<?hh

function call_from_defaults_ok(): void {
  call_into_defaults_disallowed(); // ok
}

function call_into_defaults_disallowed()[codegen]: void {
  call_from_defaults_ok(); // error
}
