<?hh

function test(mixed $x): void {
  $y = $x->foo<>;
  hh_show($y);
}

function inst_meth_equivalent(mixed $x): void {
  $y = inst_meth($x, 'foo');
  hh_show($y);
}
