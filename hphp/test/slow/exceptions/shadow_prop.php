<?hh
class MyException extends Exception
{
  protected $file = 7;
  protected $line = 'abc';
}


<<__EntryPoint>>
function main_shadow_prop() {
$exception = new MyException('Error', 1234);
echo "DONE\n";
}
