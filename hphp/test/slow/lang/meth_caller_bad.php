<?hh
class A {}
<<__EntryPoint>> function main(): void {
$cb = meth_caller('B', 'c');
$cb(new A());
}
