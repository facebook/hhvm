<?hh
class A {
    static function foo() { return 'foo'; }
}
<<__EntryPoint>> function main() {
$classname        =  'A';
$wrongClassname   =  'B';
$methodname       =  'foo';

echo $classname::$methodname()."\n";

echo $wrongClassname::$methodname()."\n";
echo "===DONE===\n";
}
