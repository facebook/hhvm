<?php

function __autoload() { throw new Exception('sup'); class nothing {} }

function main() {
  echo class_exists('nothing');
  echo "\n";
}
try { main(); } catch (Exception $x) { echo $x->getMessage(); echo "\n"; }
echo "done\n";
