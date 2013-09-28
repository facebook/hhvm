<?php
$serialized_object='O:1:"A":4:{s:1:"b";O:1:"B":0:{}s:2:"b1";r:2;s:1:"c";O:1:"B":0:{}s:2:"c1";r:4;}';
function __autoload($name) {
    unserialize("i:4;");
    eval("class $name {} ");
}

print_r(unserialize($serialized_object));
echo "okey";
?>