<?hh

function nonreturn_toplevel_void(void $x): int {
  return 1;
}

function nonreturn_toplevel_noreturn(noreturn $x): int {
  return 1;
}
