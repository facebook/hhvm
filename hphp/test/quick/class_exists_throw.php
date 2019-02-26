<?php

function __autoload() { throw new Exception('sup'); class nufin {} }

function main() {
  echo class_exists('nufin');
  echo "\n";
}
try { main(); } catch (Exception $x) { echo $x->getMessage(); echo "\n"; }
echo "done\n";
