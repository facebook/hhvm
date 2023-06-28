<?hh
class A {
  <<__DynamicallyCallable>> static function foo() :mixed{ return 'foo'; }
}
<<__EntryPoint>> function main(): void {
$classname        =  'A';
$wrongClassname   =  'B';
$methodname       =  'foo';

echo $classname::$methodname()."\n";

echo $wrongClassname::$methodname()."\n";
echo "===DONE===\n";
}
