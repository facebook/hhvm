<?hh
class T {
  public $str;
  public $int;
  function __construct() {
    $this->str = '';
    $this->int = 0;
  }
  function bongo($a, $b) :mixed{
    $this->str .= $a;
    $this->int += $b;
  }
}
<<__EntryPoint>> function main(): void {
error_reporting(0);

print "Test begin\n";

$arr = vec[0];
$arr[0] += 23;

var_dump($arr);

$t = new T();
$t->bongo("eine", 1);
$t->bongo(":zwei", 2);
var_dump($t);

$a = vec[5];
$zero = 0;
$a[$zero] += 1;
var_dump($a);

try {
  $a[17] += 1;
} catch (Exception $e) {
  print(get_class($e).': '.$e->getMessage()."\n");
}

print "Test end\n";
}
