<?php

function __autoload() { throw new Exception('sup'); class nothing {} }
class dtor { public function __destruct() { echo "dtor\n"; } }

function main() {
  $x = new dtor;
  echo $x + class_exists('nothing');
  echo "\n";
}
try { main(); } catch (Exception $x) { echo $x->getMessage(); echo "\n"; }
echo "done\n";

