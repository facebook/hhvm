<?php
class foo { 	
  function __construct($arrayobj) { 
    var_dump($arrayobj);
  } 
} 

new foo(array(new stdClass)); 
?>
