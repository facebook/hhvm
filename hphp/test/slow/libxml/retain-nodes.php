<?hh
// c.f. http://3v4l.org/sbf0s

function foo() :mixed{
  $d = new DOMDocument;
  $a = $d->createElement('foo', 'bar');
  $b = $d->createElement('fiz', 'buz');
  $c = $d->createElement('one', 'two');

  $d->appendChild($a);
  $a->appendChild($b);
  $b->appendChild($c);

  return $c;
}
<<__EntryPoint>>
function main(): void {
  $x = foo();
  while ($x) {
    try {
      var_dump($x->tagName);
    } catch (UndefinedPropertyException $e) {
      var_dump($e->getMessage());
    }
    $x = $x->parentNode;
  }
}
