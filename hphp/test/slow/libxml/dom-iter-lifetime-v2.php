<?php
// c.f. http://3v4l.org/S4isH

function foo() {
  $d = new DOMDocument;
  $c = $d->createDocumentFragment();
  $g = $d->createElement('fiz', 'buz');
  $h = $d->createElement('red', 'xen');

  $c->appendChild($g);
  $c->appendChild($h);

  return $c->childNodes;
}

foreach (foo() as $x) {
  var_dump($x->nodeValue);
}
var_dump($x->nodeValue);
