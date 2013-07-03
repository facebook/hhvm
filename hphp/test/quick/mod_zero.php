<?php

function one() {
  return 1;
}

function main() {
  var_dump(one() % 0);
}
main();
