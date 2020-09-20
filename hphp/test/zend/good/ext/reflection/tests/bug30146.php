<?hh
class test {
  static public $a = 1;
}
<<__EntryPoint>> function main(): void {
$r = new ReflectionProperty('test', 'a');
var_dump($r->getValue(null));

$r->setValue(NULL, 2);
var_dump($r->getValue());

$r->setValue(3);
var_dump($r->getValue());
echo "===DONE===\n";
}
