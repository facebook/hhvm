<?php

function main() {
  $root = new DOMElement('html');
  $copy = clone $root;
}

main();
echo "Done.\n";
