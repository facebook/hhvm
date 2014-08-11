<?php
namespace ns;
function func(){}
new \ReflectionFunction('ns\func');
new \ReflectionFunction('\ns\func');
echo "Ok\n"
?>
