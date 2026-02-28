<?hh

class Y {
  private $priv = 'priv';
  protected $prot = 'prot';
}
class Z extends Y {
}

<<__EntryPoint>>
function main_1547() :mixed{
$x = new Z;
$s = serialize($x);
$x = unserialize($s);
var_dump($x);
var_dump(serialize($x));
}
