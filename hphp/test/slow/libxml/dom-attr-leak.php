<?php

function makeBlob($length) {
  $str = '';
  for ($i = 0; $i < $length; $i++) {
    $str .= chr(rand(ord('a'), ord('z')));
  }
  return $str;
}

function foo() {
  $doc = new DOMDocument;
  $doc->loadHTML('<html><a id="foo" blob="'.makeBlob(500).'">foo</a></html>');
  $link = $doc->getElementById('foo');
  for ($i=0; $i<10; $i++) {
    $blob = makeBlob(500);
    $attr = $doc->createAttribute('blob');

    $attr->appendChild($doc->createTextNode($blob . '1'));
    $attr->appendChild($doc->createTextNode($blob . '2'));
    $attr->appendChild($doc->createTextNode($blob . '3'));
    $attr->appendChild($doc->createTextNode($blob . '4'));
    $attr->appendChild($doc->createTextNode($blob . '5'));

    $link->appendChild($attr);
  }
}

function runfoo() {
  $start = memory_get_usage(true);
  for ($i=0; $i<10; $i++) {
    foo();
  }
  $end = memory_get_usage(true);
  return $end - $start;
}

foo();
foo();
runfoo();
runfoo();
echo runfoo(), "\n";
