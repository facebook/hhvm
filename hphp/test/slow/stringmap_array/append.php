<?php

function cow_append($arr) {
  $arr[] = "warning";
}

function main() {
  $a = hphp_msarray();
  $a[] = "warning";
  $a[] = "no warning";

  $a = hphp_msarray();
  cow_append($a);
  $a[] = "warning";
}

main();
