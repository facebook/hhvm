<?php

trait t {
 }
class C {
 use t;
 }
var_dump(class_exists('C'));
