<?hh

function foo() :mixed{
  $html = '<b>Hello</b><i>World</i>';
  $doc = new DOMDocument();
  $element = $doc->createDocumentFragment();
  $element->appendXML($html);
  foreach ($element->childNodes->getIterator() as $child) {
    $element = null;
    $doc = null;
    var_dump($child->nodeValue);
  }
}

<<__EntryPoint>>
function main_1680() :mixed{
foo();
}
