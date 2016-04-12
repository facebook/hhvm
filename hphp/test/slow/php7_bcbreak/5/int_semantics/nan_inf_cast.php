<?php

function always_true() {
  // Try to defeat any possible inlining / constant folding.
  return mt_rand(1, 2) < 10;
}

function a_nan() {
  return (int)NAN;
}

function b_nan() {
  $x = NAN;
  $y = (int)$x;
  return $y;
}

function get_nan() {
  if (always_true()) {
    return NAN;
  } else {
    return 0;
  }
}

function c_nan() {
  return (int)get_nan();
}

function a_inf() {
  return (int)INF;
}

function b_inf() {
  $x = INF;
  $y = (int)$x;
  return $y;
}

function get_inf() {
  if (always_true()) {
    return INF;
  } else {
    return 0;
  }
}

function c_inf() {
  return (int)get_inf();
}

var_dump(a_nan());
var_dump(b_nan());
var_dump(c_nan());
var_dump(a_inf());
var_dump(b_inf());
var_dump(c_inf());
