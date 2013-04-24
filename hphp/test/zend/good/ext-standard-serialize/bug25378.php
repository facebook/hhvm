<?php 
var_dump(unserialize("s:-1:\"\";"));
var_dump(unserialize("i:823"));
var_dump(unserialize("O:8:\"stdClass :0:{}"));
var_dump(unserialize("O:8:\"stdClass\"+0:{}"));
var_dump(unserialize("O:1000:\"stdClass\":0:{}"));
var_dump(unserialize("a:2:{i:0;s:2:\"12\":"));
var_dump(unserialize("a:2:{i:0;s:2:\"12\";i:1;s:3000:\"123"));
var_dump(unserialize("a:2:{i:0;s:2:\"12\"+i:1;s:3:\"123\";}"));
var_dump(unserialize("a:2:{i:0;s:2:\"12\";i:1;s:3:\"123\";"));
var_dump(unserialize("s:3000:\"123\";"));
var_dump(unserialize("s:3000:\"123"));
var_dump(unserialize("s:3:\"123;"));
var_dump(unserialize("s:0:\"123\";"));
?>
===DONE===