<?php

function main() {
  do {
    var_dump("top");
    continue;
    var_dump("bottom");
  } while (false);
}
main();
