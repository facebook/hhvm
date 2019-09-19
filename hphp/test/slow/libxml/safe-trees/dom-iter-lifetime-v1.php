<?hh
// c.f. http://3v4l.org/qa6EP

function foo() {

  LibxmlSafeTreesDomIterLifetimeV1::$d = new DOMDocument;
  $c = LibxmlSafeTreesDomIterLifetimeV1::$d->createDocumentFragment();
  $g = LibxmlSafeTreesDomIterLifetimeV1::$d->createElement('fiz', 'buz');
  $h = LibxmlSafeTreesDomIterLifetimeV1::$d->createElement('red', 'xen');

  $c->appendChild($g);
  $c->appendChild($h);

  return $c->childNodes;
}

abstract final class LibxmlSafeTreesDomIterLifetimeV1 {
  public static $d;
}
<<__EntryPoint>>
function main_entry(): void {

  foreach (foo() as $x) {
    var_dump($x->nodeValue);
  }
  var_dump($x->nodeValue);
  var_dump(get_class(LibxmlSafeTreesDomIterLifetimeV1::$d));
}
