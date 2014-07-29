<?php

function main() {
  $a = hphp_msarray();
  idx($a, "no warning");
  idx($a, 10); // warning
}

main();
