<?hh

class foo{
  public $public = 'public';
  public function __sleep()  :mixed{
 return vec['public'];
 }
}

<<__EntryPoint>>
function main_753() :mixed{
$foo = new foo();
$data = serialize($foo);
var_dump($data);
}
