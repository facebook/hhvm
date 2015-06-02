<?php
// c.f. http://3v4l.org/sbf0s

function foo() {
  $d = new DOMDocument;
  $a = $d->createElement('foo', 'bar');
  $b = $d->createElement('fiz', 'buz');
  $c = $d->createElement('one', 'two');

  $d->appendChild($a);
  $a->appendChild($b);
  $b->appendChild($c);

  return $c;
}

$x = foo();
while ($x) {
  var_dump($x->tagName);
  $x = $x->parentNode;
}
