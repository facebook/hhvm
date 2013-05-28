<?php

function not_a_closure() {
   return 1;
 }
;
 $rf = new ReflectionFunction('not_a_closure');
 var_dump($rf->isClosure());
 var_dump($rf->isGenerator());
  function is_a_generator() {
   yield 1;
   yield 2;
 }
;
 $rf = new ReflectionFunction('is_a_generator');
 var_dump($rf->isClosure());
 var_dump($rf->isGenerator());
  $cl = function() {
   return 1;
 }
;
 $rf = new ReflectionFunction($cl);
 var_dump($rf->isClosure());
 var_dump($rf->isGenerator());
  $cl = function() {
   yield 1;
   yield 2;
 }
;
 $rf = new ReflectionFunction($cl);
 var_dump($rf->isClosure());
 var_dump($rf->isGenerator());

