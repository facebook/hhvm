<?hh

function g($func) {
  return $func('x');
}

class StrRef {
  public function __construct(public string $val) {}
}

<<__EntryPoint>>
function main_clone() {
  $y = 'y';
  $ref = new StrRef('');
  $f = function($x) use ($y, $ref) {
    $ref->val .= 'z';
    return $x . $y . $ref->val . "\n";
  };
  echo $f('x');
  echo g(clone $f);
  echo g(clone $f);
}
