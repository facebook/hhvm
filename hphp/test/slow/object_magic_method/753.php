<?hh

class foo{
  public $public = 'public';
  public function __sleep()  :mixed{
 return varray['public'];
 }
}

<<__EntryPoint>>
function main_753() :mixed{
$foo = new foo();
$data = serialize($foo);
var_dump($data);
}
