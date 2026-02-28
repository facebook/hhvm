<?hh

class foo {
  <<__DynamicallyCallable>> static public function a() :mixed{
    print "ok\n";
  }
}
<<__EntryPoint>> function main(): void {
$a = 'a';
$b = 'a';

$class = 'foo';

foo::a();
HH\dynamic_class_meth(foo::class, $a)();

$class::a();
HH\dynamic_class_meth($class, $a)();
}
