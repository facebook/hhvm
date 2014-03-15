<?php

function postInc($x) {
  return $x++;
}

function preInc($x) {
  return ++$x;
}

function postDec($x) {
  return $x--;
}

function preDec($x) {
  return --$x;
}

var_dump(postInc(2));
var_dump(preInc(2));
var_dump(postDec(2));
var_dump(preDec(2));

var_dump(postInc(2.5));
var_dump(preInc(2.5));
var_dump(postDec(2.5));
var_dump(preDec(2.5));

var_dump(postInc(false));
var_dump(preInc(false));
var_dump(postDec(false));
var_dump(preDec(false));

var_dump(postInc(true));
var_dump(preInc(true));
var_dump(postDec(true));
var_dump(preDec(true));

var_dump(postInc(null));
var_dump(preInc(null));
var_dump(postDec(null));
var_dump(preDec(null));
