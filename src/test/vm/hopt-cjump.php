<?php

function same($left, $right) {
  echo ($left === $right) ? "true\n" : "false\n";
}

function eq($left, $right) {
  echo ($left == $right) ? "true\n" : "false\n";
}

function neq($left, $right) {
  echo ($left != $right) ? "true\n" : "false\n";
}

same(false, 0);
neq(0, "b");
eq(true, -1);
