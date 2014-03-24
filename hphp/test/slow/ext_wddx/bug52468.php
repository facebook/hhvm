<?php

$message = "<wddxPacket version='1.0'><header><comment>my_command</comment></header><data><struct><var name='handle'><number></number></var></struct></data></wddxPacket>";

print_r(wddx_deserialize($message));
print_r(wddx_deserialize($message));

?>
