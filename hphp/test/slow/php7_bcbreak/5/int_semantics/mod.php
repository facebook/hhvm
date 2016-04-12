<?php

function always_true() {
  return mt_rand(1, 2) < 10;
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
  try { 1 % 0; } catch (\__SystemLib\Throwable $e) { exn($e); }
  try { 1 % 0.0; } catch (\__SystemLib\Throwable $e) { exn($e); }
  try { 1 % noinline(0); } catch (\__SystemLib\Throwable $e) { exn($e); }
  try { 1 % noinline(0.0); } catch (\__SystemLib\Throwable $e) { exn($e); }
  try { noinline(1) % 0; } catch (\__SystemLib\Throwable $e) { exn($e); }
  try { noinline(1) % 0.0; } catch (\__SystemLib\Throwable $e) { exn($e); }
  try { noinline(1) % noinline(0); }
    catch (\__SystemLib\Throwable $e) { exn($e); }
  try { noinline(1) % noinline(0.0); }
    catch (\__SystemLib\Throwable $e) { exn($e); }
}

run_tests();
