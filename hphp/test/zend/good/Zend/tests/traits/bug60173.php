<?hh

trait foo { }
<<__EntryPoint>> function main(): void {
$rc = new ReflectionClass('foo');
$rc->newInstance();
}
