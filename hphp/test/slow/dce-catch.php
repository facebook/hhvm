<?hh

class C {}

function f($x) {
  try {
    return $x->missing;
  } catch (Exception $e) {
    return null;
  }
}

<<__EntryPoint>>
function main() {
  $x = dict['a' => 'b', 'c' => f(new C())];
  var_dump($x['a']);
}
