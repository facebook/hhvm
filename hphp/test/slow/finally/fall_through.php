<?php

function main() {
  try {
    var_dump("try");
  } finally {
    var_dump("finally");
  }
}
main();
