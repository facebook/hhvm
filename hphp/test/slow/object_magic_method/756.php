<?hh

class foo{
  public $public = 'public';
  public function __wakeup()  :mixed{
 echo "__wakeup called\n";
 return 1;
 }
}

<<__EntryPoint>>
function main_756() :mixed{
$foo = unserialize("O:3:\"foo\":1:{s:6:\"public\";s:6:\"public\";}");
var_dump($foo);
}
