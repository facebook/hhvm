<?hh

class V { const X = 10; }
function values() {
  $VALUES = array(
    'X'                        => V::X,
  );
  return $VALUES;
}
function foo() { var_dump(values()['X']); }
foo();
foo();
