<?php

function elemNoPromo() {
  $ret = " ";
  $ret[0] = 'A';
  return $ret;
}

function elemPromo() {
  $ret = "";
  $ret[0] = 'A';
  return $ret;
}

function propNoPromo() {
  $ret = " ";
  $ret->prop = 'A';
  return $ret;
}

function propPromo() {
  $ret = "";
  $ret->prop = 'A';
  return $ret;
}

var_dump(elemPromo());
var_dump(elemPromo());
var_dump(elemNoPromo());
var_dump(elemNoPromo());
var_dump(propPromo());
var_dump(propPromo());
var_dump(propNoPromo());
var_dump(propNoPromo());
