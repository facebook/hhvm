<?hh

function call_from_defaults_ok(): void {
  call_into_defaults_allowed(); // ok
}

function call_from_codegen_ok()[codegen]: void {
  call_into_defaults_allowed(); // ok
}

function call_into_defaults_allowed()[codegen_unsafe]: void {
  call_from_defaults_ok(); // ok
}
