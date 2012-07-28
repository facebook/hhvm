<?php
error_reporting(0);

print "Test begin\n";

$arr = array(0);
$arr[0] += 23;
$arr[1] += 47;

$arr[] .= 1;
$arr[] += 1;
$arr[] -= 1;

var_dump($arr);

class T {
  public $str;
  public $int;
  function __construct() {
    $this->str = '';
    $this->int = 0;
  }
  function bongo($a, $b) {
    $this->str .= $a;
    $this->int += $b;
  }
}

$t = new T();
$t->bongo("eine", 1);
$t->bongo(":zwei", 2);
var_dump($t);

$a = array(5);
$zero = 0;
$a[$zero] += 1;
var_dump($a);


$x = 40;
$arr = array();
$arr[0] =& $x;
$arr[0] += 4;
var_dump($x);


print "Test end\n";

