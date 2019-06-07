<?hh

trait foo { }
<<__EntryPoint>> function main() {
$rc = new ReflectionClass('foo');
$rc->newInstance();
}
