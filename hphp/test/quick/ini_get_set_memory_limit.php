<?php

var_dump( ini_get('memory_limit'));

ini_set('memory_limit', '128M');
var_dump( ini_get('memory_limit'));

ini_set('memory_limit', '128G');
var_dump( ini_get('memory_limit'));

ini_set('memory_limit', '128K');
var_dump( ini_get('memory_limit'));

ini_set('memory_limit', '136314880');
var_dump( ini_get('memory_limit'));
