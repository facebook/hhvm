<?hh

// Extend a class with NativeData
class C<reify T> extends SimpleXMLElement {
  function f() {
    var_dump(HH\ReifiedGenerics\getTypeStructure<T>());
  }
}
<<__EntryPoint>> function main(): void {
$c = new C<int>("<hi></hi>");
$c->f();

$d = clone($c);
$d->f();
}
