<?php

function main() {
  echo "0";
  ob_start();
  echo "1";
  ob_start();
  echo "2";
  ob_end_flush();
  echo "3";
  ob_end_flush();
  echo "4";
}

main();
