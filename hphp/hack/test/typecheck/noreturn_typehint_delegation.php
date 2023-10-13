<?hh

function nr(): noreturn {
  throw new Exception('nope');
}

function nr_delegation(): noreturn {
  return nr();
}

function nr_usage(): noreturn {
  nr(); // this depends on terminality calculation
}
