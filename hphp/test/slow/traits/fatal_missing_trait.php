<?php

if (isset($g)) {
  trait T {}
}

class C {
  use T;
}

function main() {
  var_dump(new C);
}

main();
