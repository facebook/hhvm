<?hh

function dyn(): dynamic { return "4" as dynamic; }

function my_vec<T>(T $a, T $b) : vec<T> {
  return vec[$a, $b];
}

function test1(): vec<dynamic> {
  return vec[1, dyn()];
}

function test2(): vec<dynamic> {
  return my_vec(1, dyn());
}

function expectVecDyn(vec<dynamic> $vd):void { }

function test3():void {
  expectVecDyn(vec[5]);
}
