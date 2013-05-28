<?php

var_dump(class_exists('C'));
class C implements Countable {
 function count() {
 return 0;
 }
 }
