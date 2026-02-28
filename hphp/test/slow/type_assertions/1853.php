<?hh

function g(inout $x) :mixed{
 var_dump($x);
}
function f($x) :mixed{
  if (is_array($x)) {

    var_dump($x);
    try {
      var_dump($x[0]);
      if (is_array($x[0])) var_dump($x[0][1]);
    } catch (Exception $e) { echo $e->getMessage()."\n"; }
  }
  if (is_array($x) && $x) {
    g(inout $x);
  }
}

<<__EntryPoint>>
function main_1853() :mixed{
f(vec[]);
f(vec[0, 1]);
f(vec[dict[1 => 1]]);
}
