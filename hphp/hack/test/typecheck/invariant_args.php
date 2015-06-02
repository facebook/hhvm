<?hh

abstract class C {
  abstract public function y(): int;
}

function opt_c(): ?C {
  return null;
}

function f() {
  $c = opt_c();
  hh_show($c);
  invariant($c instanceof C, 'format: '.($c?->y()));
  hh_show($c);
}
