<?php

function always_true() {
  return mt_rand(1, 2) < 10;
}

function id($x) {
  return $x;
}

function noinline($x) {
  if (always_true()) {
    return $x;
  } else {
    return null;
  }
}

function inf_tests() {
  echo 1 / 0, "\n";
  echo 1 / id(0), "\n";
  echo 1 / noinline(0), "\n";
  echo id(1) / 0, "\n";
  echo id(1) / id(0), "\n";
  echo id(1) / noinline(0), "\n";
  echo noinline(1) / 0, "\n";
  echo noinline(1) / id(0), "\n";
  echo noinline(1) / noinline(0), "\n";

  echo 1 / 0.0, "\n";
  echo 1 / id(0.0), "\n";
  echo 1 / noinline(0.0), "\n";
  echo id(1) / 0.0, "\n";
  echo id(1) / id(0.0), "\n";
  echo id(1) / noinline(0.0), "\n";
  echo noinline(1) / 0.0, "\n";
  echo noinline(1) / id(0.0), "\n";
  echo noinline(1) / noinline(0.0), "\n";

  echo 1.0 / 0, "\n";
  echo 1.0 / id(0), "\n";
  echo 1.0 / noinline(0), "\n";
  echo id(1.0) / 0, "\n";
  echo id(1.0) / id(0), "\n";
  echo id(1.0) / noinline(0), "\n";
  echo noinline(1.0) / 0, "\n";
  echo noinline(1.0) / id(0), "\n";
  echo noinline(1.0) / noinline(0), "\n";

  echo 1.0 / 0.0, "\n";
  echo 1.0 / id(0.0), "\n";
  echo 1.0 / noinline(0.0), "\n";
  echo id(1.0) / 0.0, "\n";
  echo id(1.0) / id(0.0), "\n";
  echo id(1.0) / noinline(0.0), "\n";
  echo noinline(1.0) / 0.0, "\n";
  echo noinline(1.0) / id(0.0), "\n";
  echo noinline(1.0) / noinline(0.0), "\n";
}

function neginf_tests() {
  echo -1 / 0, "\n";
  echo -1 / id(0), "\n";
  echo -1 / noinline(0), "\n";
  echo id(-1) / 0, "\n";
  echo id(-1) / id(0), "\n";
  echo id(-1) / noinline(0), "\n";
  echo noinline(-1) / 0, "\n";
  echo noinline(-1) / id(0), "\n";
  echo noinline(-1) / noinline(0), "\n";

  echo -1 / 0.0, "\n";
  echo -1 / id(0.0), "\n";
  echo -1 / noinline(0.0), "\n";
  echo id(-1) / 0.0, "\n";
  echo id(-1) / id(0.0), "\n";
  echo id(-1) / noinline(0.0), "\n";
  echo noinline(-1) / 0.0, "\n";
  echo noinline(-1) / id(0.0), "\n";
  echo noinline(-1) / noinline(0.0), "\n";

  echo -1.0 / 0, "\n";
  echo -1.0 / id(0), "\n";
  echo -1.0 / noinline(0), "\n";
  echo id(-1.0) / 0, "\n";
  echo id(-1.0) / id(0), "\n";
  echo id(-1.0) / noinline(0), "\n";
  echo noinline(-1.0) / 0, "\n";
  echo noinline(-1.0) / id(0), "\n";
  echo noinline(-1.0) / noinline(0), "\n";

  echo -1.0 / 0.0, "\n";
  echo -1.0 / id(0.0), "\n";
  echo -1.0 / noinline(0.0), "\n";
  echo id(-1.0) / 0.0, "\n";
  echo id(-1.0) / id(0.0), "\n";
  echo id(-1.0) / noinline(0.0), "\n";
  echo noinline(-1.0) / 0.0, "\n";
  echo noinline(-1.0) / id(0.0), "\n";
  echo noinline(-1.0) / noinline(0.0), "\n";
}

function negzero_tests() {
  echo 1 / (-1 * 0.0), "\n";
  echo 1 / id(-1 * 0.0), "\n";
  echo 1 / noinline(-1 * 0.0), "\n";
  echo id(1) / (-1 * 0.0), "\n";
  echo id(1) / id(-1 * 0.0), "\n";
  echo id(1) / noinline(-1 * 0.0), "\n";
  echo noinline(1) / (-1 * 0.0), "\n";
  echo noinline(1) / id(-1 * 0.0), "\n";
  echo noinline(1) / noinline(-1 * 0.0), "\n";

  echo 1.0 / (-1 * 0.0), "\n";
  echo 1.0 / id(-1 * 0.0), "\n";
  echo 1.0 / noinline(-1 * 0.0), "\n";
  echo id(1.0) / (-1 * 0.0), "\n";
  echo id(1.0) / id(-1 * 0.0), "\n";
  echo id(1.0) / noinline(-1 * 0.0), "\n";
  echo noinline(1.0) / (-1 * 0.0), "\n";
  echo noinline(1.0) / id(-1 * 0.0), "\n";
  echo noinline(1.0) / noinline(-1 * 0.0), "\n";
}

function nan_tests() {
  echo 0 / 0, "\n";
  echo 0 / id(0), "\n";
  echo 0 / noinline(0), "\n";
  echo id(0) / 0, "\n";
  echo id(0) / id(0), "\n";
  echo id(0) / noinline(0), "\n";
  echo noinline(0) / 0, "\n";
  echo noinline(0) / id(0), "\n";
  echo noinline(0) / noinline(0), "\n";

  echo 0 / 0.0, "\n";
  echo 0 / id(0.0), "\n";
  echo 0 / noinline(0.0), "\n";
  echo id(0) / 0.0, "\n";
  echo id(0) / id(0.0), "\n";
  echo id(0) / noinline(0.0), "\n";
  echo noinline(0) / 0.0, "\n";
  echo noinline(0) / id(0.0), "\n";
  echo noinline(0) / noinline(0.0), "\n";

  echo 0.0 / 0, "\n";
  echo 0.0 / id(0), "\n";
  echo 0.0 / noinline(0), "\n";
  echo id(0.0) / 0, "\n";
  echo id(0.0) / id(0), "\n";
  echo id(0.0) / noinline(0), "\n";
  echo noinline(0.0) / 0, "\n";
  echo noinline(0.0) / id(0), "\n";
  echo noinline(0.0) / noinline(0), "\n";

  echo 0.0 / 0.0, "\n";
  echo 0.0 / id(0.0), "\n";
  echo 0.0 / noinline(0.0), "\n";
  echo id(0.0) / 0.0, "\n";
  echo id(0.0) / id(0.0), "\n";
  echo id(0.0) / noinline(0.0), "\n";
  echo noinline(0.0) / 0.0, "\n";
  echo noinline(0.0) / id(0.0), "\n";
  echo noinline(0.0) / noinline(0.0), "\n";
}

inf_tests();
neginf_tests();
negzero_tests();
nan_tests();
