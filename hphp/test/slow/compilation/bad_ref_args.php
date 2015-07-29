<?php

function main() {
  $a = "foo";
  var_dump(socket_select($a, $a, $a, 42));
  var_dump(sscanf("42", "%d", new stdclass));
}

main();
