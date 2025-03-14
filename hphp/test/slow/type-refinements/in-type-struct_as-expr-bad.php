<?hh

interface Box {
  abstract const type T;
}

function callee(mixed $b): void {
  $_ = $b as Box with { type T = int }; // FATAL
}

<<__EntryPoint>>
function caller(): void {
  callee(new IntBox());
}

class IntBox implements Box {
  const type T = int;
}
