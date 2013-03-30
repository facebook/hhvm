<?php

class Test implements ArrayAccess
{
        public function __construct() { }       
        public function offsetExists( $offset ) { return false; }
        public function offsetGet( $offset ) { return $offset; }
        public function offsetSet( $offset, $data ) { }
        public function offsetUnset( $offset ) { }
}

$post = new Test;
$id = 'page';
echo $post[$id.'_show'];
echo "\n";

?>
===DONE===