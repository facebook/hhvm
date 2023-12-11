<?hh

function main() :mixed{
  $a = new A;
  $a->e = 1;
  $s = serialize($a);
  var_dump($s);
  var_dump(unserialize($s));
}
class A {
  public $b;
  protected $c;
  private $d;
  public function __sleep() :mixed{
    return vec['b', 'c', 'd'];
  }
}

<<__EntryPoint>>
function main_with_sleep() :mixed{
main();
}
