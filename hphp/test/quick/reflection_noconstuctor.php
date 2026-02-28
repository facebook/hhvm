<?hh
class foo {

  public function __construct() {
    print "this should only be printed three times!\n";
  }

}
<<__EntryPoint>> function main(): void {
$c1 = new foo();
$c2 = (new ReflectionClass('foo'))->newInstance();
$c3 = (new ReflectionClass('foo'))->newInstanceArgs(vec[]);
$no_c = (new ReflectionClass('foo'))->newInstanceWithoutConstructor();
}
