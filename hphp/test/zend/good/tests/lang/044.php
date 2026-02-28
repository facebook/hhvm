<?hh
class A {
  <<__DynamicallyCallable>> static function foo() :mixed{ return 'foo'; }
}
<<__EntryPoint>> function main(): void {
$classname        =  'A';
$wrongClassname   =  'B';
$methodname       =  'foo';

echo HH\dynamic_class_meth($classname, $methodname)()."\n";

echo HH\dynamic_class_meth($wrongClassname,$methodname)()."\n";
echo "===DONE===\n";
}
