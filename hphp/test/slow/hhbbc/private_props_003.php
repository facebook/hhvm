<?hh

/*
 * Unserialization does not check values, so property types do not
 * infer specific values.
 *
 * Test this by making sure that we don't constant propagate from
 * values in object properties.
 */

class SerDe {
  private $x = 42;
  private $y = 3.14;
  private $z = false;
  public function foo() {
    $xx = $this->x + 1;
    $yy = $this->y + 1.0;
    $zz = !$this->z;
    var_dump($xx, $yy, $zz);
  }
}

function main() {
  $x = new SerDe;
  $str = serialize($x);
  $str = preg_replace('/i:42/', 'i:0', $str);
  $str = preg_replace('/d:3.14/', 'd:1.0', $str);
  $str = preg_replace('/b:0/', 'b:1', $str);
  $x = unserialize($str);
  $x->foo();
}

main();
