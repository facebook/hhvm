<?hh
class A {}
<<__EntryPoint>> function main(): void {
$cb = meth_caller(B::class, 'c');
$cb(new A());
}
