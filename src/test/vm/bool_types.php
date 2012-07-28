<?php

function main() {
  // Suppress warnings
  error_reporting(0);

  $a = "abc";
  $a[2] = "d";

  if (!"strings") {
    printf("Strings as bools is broken!");
  }
  if ($a == "abd") {
  } else {
    printf("Eq :: String -> String -> Bool broken? $a\n");
  }
  if ($a != "abd") {
    printf("Neq :: String -> String -> Bool broken? $a\n");
  }
  if ("" == null) {
    printf("empty string == null\n");
  }
  if ("0" == null) {
    printf("numeric string 0 == null\n");
  }
  if (0 == null) {
    printf("integral 0 == null\n");
  }
  if (false == null) {
    printf("boolean false == null\n");
  }
  if (1 === 2) {
    printf("check more than types, please\n");
  }
  if (1 !== 1) {
    printf("NSame broken\n");
  }
  if (!(1 === 1)) {
    printf("Same broken\n");
  }
  if (0 != $q) {
    printf("Uninitialized null vs. integer 0\n");
  }
  if (1 == $q) {
    printf("Uninitialized null vs. integer 1\n");
  }
  if (null !== $q) {
    printf("Uninitialized null vs. null NSame\n");
  }
  if (!(null === $q)) {
    printf("Uninitialized null vs. null Same\n");
  }
}
main();
