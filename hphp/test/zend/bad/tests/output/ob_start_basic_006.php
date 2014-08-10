<?php
/* 
 * proto bool ob_start([ string|array user_function [, int chunk_size [, bool erase]]])
 * Function is implemented in main/output.c
*/ 

function f($string) {
	static $i=0;
	$i++;
	$len = strlen($string);
	return "f[call:$i; len:$len] - $string\n";
}

Class C {
	public $id = 'none';

	function __construct($id) {
		$this->id = $id;
	}

	static function g($string) {
		static $i=0;
		$i++;
		$len = strlen($string);
		return "C::g[call:$i; len:$len] - $string\n";
	}
	
	function h($string) {
		static $i=0;
		$i++;
		$len = strlen($string);
		return "C::h[call:$i; len:$len; id:$this->id] - $string\n";
	}
}

function checkAndClean() {
  print_r(ob_list_handlers());
  while (ob_get_level()>0) {
    ob_end_flush();
  }
}

echo "\n ---> Test arrays: \n";
var_dump(ob_start(array("f")));
checkAndClean();

var_dump(ob_start(array("f", "f")));
checkAndClean();

var_dump(ob_start(array("f", "C::g", "f", "C::g")));
checkAndClean();

var_dump(ob_start(array("f", "non_existent", "f")));
checkAndClean();

var_dump(ob_start(array("f", "non_existent", "f", "f")));
checkAndClean();

$c = new c('originalID');
var_dump(ob_start(array($c, "h")));
checkAndClean();

var_dump(ob_start(array($c, "h")));
$c->id = 'changedID';
checkAndClean();

$c->id = 'changedIDagain';
var_dump(ob_start(array('f', 'C::g', array(array($c, "g"), array($c, "h")))));
checkAndClean();
?>
