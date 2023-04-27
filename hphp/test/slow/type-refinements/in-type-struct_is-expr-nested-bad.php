<?hh

<<file: __EnableUnstableFeatures('type_refinements')>>

interface Box {
  abstract const type T;
}

function callee(mixed $b): void {
  $_ = $b is shape('bad' => Box with { type T = int }); // FATAL
}

<<__EntryPoint>>
function caller(): void {
  callee(new IntBox());
}

class IntBox implements Box {
  const type T = int;
}
