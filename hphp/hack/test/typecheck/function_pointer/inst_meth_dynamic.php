<?hh

// Not erroring here matches its inst_meth equivalent
function test(dynamic $x): void {
  $y = $x->foo<>;
  hh_show($y);
}

function inst_meth_equivalent(dynamic $x): void {
  $y = inst_meth($x, 'foo');
  hh_show($y);
}
