<?php

function postInc(&$x) {
  return $x++;
}

function preInc(&$x) {
  return ++$x;
}

function postDec(&$x) {
  return $x--;
}

function preDec(&$x) {
  return --$x;
}

$x = 2;
var_dump(postInc($x));
var_dump($x);
var_dump(preInc($x));
var_dump($x);
var_dump(postDec($x));
var_dump($x);
var_dump(preDec($x));
var_dump($x);

$y = 2.5;
var_dump(postInc($y));
var_dump($y);
var_dump(preInc($y));
var_dump($y);
var_dump(postDec($y));
var_dump($y);
var_dump(preDec($y));
var_dump($y);

$f = false;
var_dump(postInc($f));
var_dump($f);
var_dump(preInc($f));
var_dump($f);
var_dump(postDec($f));
var_dump($f);
var_dump(preDec($f));
var_dump($f);

$t = true;
var_dump(postInc($t));
var_dump($t);
var_dump(preInc($t));
var_dump($t);
var_dump(postDec($t));
var_dump($t);
var_dump(preDec($t));
var_dump($t);
