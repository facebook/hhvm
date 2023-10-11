<?hh

function cast_int(int $x): nonnull {
  return $x;
}

function cast_bool(int $x): nonnull {
  return $x;
}

function cast_float(float $x): nonnull {
  return $x;
}

function cast_string(string $x): nonnull {
  return $x;
}

function cast_resource(resource $x): nonnull {
  return $x;
}

function cast_num(num $x): nonnull {
  return $x;
}

function cast_arraykey(arraykey $x): nonnull {
  return $x;
}

function forever(): noreturn {
  forever();
}

function f(): nonnull {
  return forever();
}
