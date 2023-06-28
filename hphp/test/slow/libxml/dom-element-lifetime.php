<?hh
// c.f. https://github.com/facebook/hhvm/issues/3096

function one() :mixed{
  $document = new DOMDocument();
  $root = new DOMElement('root');
  $document->appendChild($root);

  $child = new DOMElement('child');

  $root->appendChild($child);
  $root->removeChild($child);

  var_dump($root->C14N());
}

function two() :mixed{
  $root = new DOMElement('root');
  $child = new DOMElement('child');
  $root->appendChild($child);

  var_dump($root->C14N());
}
<<__EntryPoint>> function main(): void {
one();
two();
}
