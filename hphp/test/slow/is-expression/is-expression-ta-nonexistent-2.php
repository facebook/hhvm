<?hh

abstract class C {
  abstract const type Tnonexistent;
  function is_nonexistent(mixed $x): void {
    if ($x is this::Tnonexistent) {
      echo "unreached\n";
    } else {
      echo "yep\n";
    }
  }
}

class D extends C {
  const type Tnonexistent = shape('f' => BlerpityBlerp);
}

<<__EntryPoint>>
function main_is_expression_ta_nonexistent() :mixed{
  (new D)->is_nonexistent(new stdClass());
}
