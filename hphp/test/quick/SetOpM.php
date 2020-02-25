<?hh
class T {
  public $str;
  public $int;
  function __construct() {
    $this->str = '';
    $this->int = 0;
  }
  function bongo($a, $b) {
    $this->str .= $a;
    $this->int += $b;
  }
}
<<__EntryPoint>> function main(): void {
error_reporting(0);

print "Test begin\n";

$arr = varray[0];
$arr[0] += 23;

var_dump($arr);

$t = new T();
$t->bongo("eine", 1);
$t->bongo(":zwei", 2);
var_dump($t);

$a = varray[5];
$zero = 0;
$a[$zero] += 1;
var_dump($a);

print "Test end\n";
}
