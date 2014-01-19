<?php

$c = new UConverter('utf-8', 'latin1');
var_dump($c->convert("This is an ascii string"));
var_dump(urlencode($c->convert("Espa\xF1ol")));
var_dump(urlencode($c->convert("Stra\xDFa")));
var_dump(urlencode($c->convert("Stra\xC3\x9Fa", true)));
$k = new UConverter('utf-8', 'koi8-r');
var_dump(bin2hex($k->convert("\xE4")));
