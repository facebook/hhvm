<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

function dyn(): dynamic { return "4" upcast dynamic; }

function my_vec<T>(T $a, T $b) : vec<T> {
  return vec[$a, $b];
}

function test1(): vec<dynamic> {
  return vec[1 upcast dynamic, dyn()];
}

function test2(): vec<dynamic> {
  return my_vec(1 upcast dynamic, dyn());
}

function test3():void {
  vec[5] upcast vec<dynamic>;
}
