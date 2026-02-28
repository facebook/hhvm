<?hh

<<__SupportDynamicType>>
function f(int $i) : void {
}

function likify<T>(T $t) : ~T { return $t; }

function g(dynamic $d) : void {
  $v = Vector{};
  $v[0] = likify(1);
  f($v[0]);
}
