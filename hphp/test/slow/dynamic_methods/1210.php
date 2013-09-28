<?php

class z {
  function __construct() {
 echo 'construct';
 }
  function z() {
 echo 'method';
 }
}
$z = new z;
$z->z();
