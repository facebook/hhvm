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

function exn($e) {
  echo get_class($e), ': ', $e->getMessage(), "\n";
}

function run_tests() {
  echo 3 << 64, "\n";
  echo 3 << id(64), "\n";
  echo 3 << noinline(64), "\n";
  echo id(3) << 64, "\n";
  echo id(3) << id(64), "\n";
  echo id(3) << noinline(64), "\n";
  echo noinline(3) << 64, "\n";
  echo noinline(3) << id(64), "\n";
  echo noinline(3) << noinline(64), "\n";

  echo 3 << 65, "\n";
  echo 3 << id(65), "\n";
  echo 3 << noinline(65), "\n";
  echo id(3) << 65, "\n";
  echo id(3) << id(65), "\n";
  echo id(3) << noinline(65), "\n";
  echo noinline(3) << 65, "\n";
  echo noinline(3) << id(65), "\n";
  echo noinline(3) << noinline(65), "\n";

  echo 3 >> 64, "\n";
  echo 3 >> id(64), "\n";
  echo 3 >> noinline(64), "\n";
  echo id(3) >> 64, "\n";
  echo id(3) >> id(64), "\n";
  echo id(3) >> noinline(64), "\n";
  echo noinline(3) >> 64, "\n";
  echo noinline(3) >> id(64), "\n";
  echo noinline(3) >> noinline(64), "\n";

  echo 3 >> 65, "\n";
  echo 3 >> id(65), "\n";
  echo 3 >> noinline(65), "\n";
  echo id(3) >> 65, "\n";
  echo id(3) >> id(65), "\n";
  echo id(3) >> noinline(65), "\n";
  echo noinline(3) >> 65, "\n";
  echo noinline(3) >> id(65), "\n";
  echo noinline(3) >> noinline(65), "\n";

  echo "-\n";

  echo -3 << 64, "\n";
  echo -3 << id(64), "\n";
  echo -3 << noinline(64), "\n";
  echo id(-3) << 64, "\n";
  echo id(-3) << id(64), "\n";
  echo id(-3) << noinline(64), "\n";
  echo noinline(-3) << 64, "\n";
  echo noinline(-3) << id(64), "\n";
  echo noinline(-3) << noinline(64), "\n";

  echo -3 << 65, "\n";
  echo -3 << id(65), "\n";
  echo -3 << noinline(65), "\n";
  echo id(-3) << 65, "\n";
  echo id(-3) << id(65), "\n";
  echo id(-3) << noinline(65), "\n";
  echo noinline(-3) << 65, "\n";
  echo noinline(-3) << id(65), "\n";
  echo noinline(-3) << noinline(65), "\n";

  echo -3 >> 64, "\n";
  echo -3 >> id(64), "\n";
  echo -3 >> noinline(64), "\n";
  echo id(-3) >> 64, "\n";
  echo id(-3) >> id(64), "\n";
  echo id(-3) >> noinline(64), "\n";
  echo noinline(-3) >> 64, "\n";
  echo noinline(-3) >> id(64), "\n";
  echo noinline(-3) >> noinline(64), "\n";

  echo -3 >> 65, "\n";
  echo -3 >> id(65), "\n";
  echo -3 >> noinline(65), "\n";
  echo id(-3) >> 65, "\n";
  echo id(-3) >> id(65), "\n";
  echo id(-3) >> noinline(65), "\n";
  echo noinline(-3) >> 65, "\n";
  echo noinline(-3) >> id(65), "\n";
  echo noinline(-3) >> noinline(65), "\n";

  echo "-\n";

  echo (new stdclass) >> 64, "\n";
  echo (new stdclass) << 64, "\n";

  echo "-\n";

  try {echo 3 << -1, "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo 3 << id(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo 3 << noinline(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo id(3) << -1, "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo id(3) << id(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo id(3) << noinline(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo noinline(3) << -1, "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo noinline(3) << id(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo noinline(3) << noinline(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}

  try {echo 3 >> -1, "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo 3 >> id(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo 3 >> noinline(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo id(3) >> -1, "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo id(3) >> id(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo id(3) >> noinline(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo noinline(3) >> -1, "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo noinline(3) >> id(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
  try {echo noinline(3) >> noinline(-1), "\n";}
    catch (\__SystemLib\Throwable $e) {exn($e);}
}

run_tests();
