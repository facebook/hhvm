<?hh
class X {
  private $priv = 'priv';
  protected $prot = 'prot';
}
<<__EntryPoint>>
function main(): void {
  $x = new X;
  $s = serialize($x);
  $s = str_replace('X', 'Y', $s);
  $x = unserialize($s);
  var_dump($x);
  try {
    var_dump($x->prot);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
  try {
    var_dump($x->priv);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
}
