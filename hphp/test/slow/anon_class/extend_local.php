<?php

function main() {
  class bar {}
  return new class extends bar { function foo() { var_dump(__METHOD__); } };
}

var_dump(main());
