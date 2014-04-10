<?php

function main() {
  $invalid = "\0:";
  var_dump(strpbrk('foo:bar', $invalid));
}

main();
