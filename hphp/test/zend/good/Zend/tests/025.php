<?hh

class foo {
  <<__DynamicallyCallable>> static public function a() {
    print "ok\n";
  }
}
<<__EntryPoint>> function main(): void {
$a = 'a';
$b = 'a';

$class = 'foo';

foo::a();
foo::$a();

$class::a();
$class::$a();
}
