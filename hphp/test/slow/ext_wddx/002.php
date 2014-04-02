<?php
$pkt = wddx_packet_start('TEST comment');

$var1 = NULL;
$var2 = 'some string';
$var3 = 756;
$var4 = true;

// add vars to packet
wddx_add_vars($pkt, 'var1', 'var2', array('var3', 'var4'));
echo wddx_packet_end($pkt);
