<?php

$format = "(\000)\n";

printf($format);

vprintf($format, array());

echo sprintf($format);

echo vsprintf($format, array());
