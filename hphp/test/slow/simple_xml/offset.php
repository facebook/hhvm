<?php

function main() {
  $x = new SimpleXMLElement(
    '<a><b><c>d</c></b><b><c>d</c></b></a>'
  );
  foreach ($x as $child) {
    var_dump(isset($child[0]));
    var_dump(isset($child[1]));

    var_dump((string) $child[0]);
    var_dump((string) $child[0]->c);
  }
}
main();
