<?hh

class C { static function f() :mixed{} }

function test(callable $x) :mixed{ return $x; }

<<__EntryPoint>>
function main() :mixed{
  $base = vec['C', 'f'];
  $callables = vec[
    varray($base),
    darray($base),
    vec($base),
    dict($base),
  ];

  foreach ($callables as $callable) {
    var_dump(is_callable($callable));
    var_dump(test($callable));
  }

  // Static version, so that we can test HHBBC, too.
  $callable = vec['C', 'f'];
  var_dump(is_callable($callable));
  var_dump(test($callable));
  $callable = dict[0 => 'C', 1 => 'f'];
  var_dump(is_callable($callable));
  var_dump(test($callable));
  $callable = vec['C', 'f'];
  var_dump(is_callable($callable));
  var_dump(test($callable));
  $callable = dict[0 => 'C', 1 => 'f'];
  var_dump(is_callable($callable));
  var_dump(test($callable));

  // Check that keysets are NOT callable.
  $not_callable = keyset['C', 'f'];
  var_dump(is_callable($not_callable));
}
