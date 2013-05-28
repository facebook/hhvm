<?php

function p($a) {
 print $a;
}
 register_shutdown_function('p', 'shutdown');
