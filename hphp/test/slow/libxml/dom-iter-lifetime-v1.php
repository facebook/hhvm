<?hh
// c.f. http://3v4l.org/qa6EP

function foo() {

  LibxmlDomIterLifetimeV1::$d = new DOMDocument;
  $c = LibxmlDomIterLifetimeV1::$d->createDocumentFragment();
  $g = LibxmlDomIterLifetimeV1::$d->createElement('fiz', 'buz');
  $h = LibxmlDomIterLifetimeV1::$d->createElement('red', 'xen');

  $c->appendChild($g);
  $c->appendChild($h);

  return $c->childNodes;
}

foreach (foo() as $x) {
  var_dump($x->nodeValue);
}
var_dump($x->nodeValue);
var_dump(get_class(LibxmlDomIterLifetimeV1::$d));

abstract final class LibxmlDomIterLifetimeV1 {
  public static $d;
}
