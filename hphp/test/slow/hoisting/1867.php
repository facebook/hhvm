<?php

class C implements Countable {
 function count() {
 return 0;
 }
 }
var_dump(class_exists('C'));
