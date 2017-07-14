<?php
class wpq {
    private $unreferenced;
 
    public function __get($name) {
        return $this->$name . "XXX";
    }
}
 
function ret_assoc() {
	$x = "XXX";
    return array('foo' => 'bar', $x);
}
 
$wpq = new wpq;
$wpq->interesting =& ret_assoc();
$x = $wpq->interesting;
printf("%s\n", $x);
