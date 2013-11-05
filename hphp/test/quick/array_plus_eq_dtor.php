<?php

class dtor { public function __destruct() { echo "dtor\n"; } }

function main() {
  $a = array(new dtor);
  var_dump($a+array(4,5,6));
}

main();
echo "done\n";
