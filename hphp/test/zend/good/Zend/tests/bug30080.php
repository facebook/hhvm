<?php
class foo { 	
  function foo($arrayobj) { 
    var_dump($arrayobj);
  } 
} 

new foo(array(new stdClass)); 
?>