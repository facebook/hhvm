<?php

function main() {
  $uri = realpath(__DIR__.'/xinclude-1.xml');
  $xml = file_get_contents($uri);
  $doc = new DOMDocument();
  $doc->loadXML($xml);
  $doc->documentURI = $uri;
  $doc->xinclude();
  var_dump($doc->saveXML());
}

main();
