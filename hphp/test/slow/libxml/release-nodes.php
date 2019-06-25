<?hh
// c.f. http://3v4l.org/8Peti

function foo() {
  $d = new DOMDocument;
  $a = $d->createElement('foo', 'bar');
  $b = $d->createElement('fiz', 'buz');
  $c = $d->createElement('one', 'two');

  $a->appendChild($b);
  $b->appendChild($c);

  // DOMDocument frees elements when they are not referenced. So we need to keep
  // $a alive until here.
  __hhvm_intrinsics\launder_value($a);

  return $c;
}
<<__EntryPoint>> function main(): void {
$x = foo();
while ($x) {
  var_dump($x->tagName);
  $x = $x->parentNode;
}
}
