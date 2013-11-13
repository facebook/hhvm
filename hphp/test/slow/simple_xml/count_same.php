<?php

function main() {
  $x = new SimpleXMLElement(
    '<a><b/><b/><c/></a>'
  );
  var_dump($x->children()->count());
}
main();
