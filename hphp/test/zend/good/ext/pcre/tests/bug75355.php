<?php

var_dump(preg_quote('#'));

var_dump(preg_match('~^(' . preg_quote('hello#world', '~') . ')\z~x', 'hello#world', &$m));

var_dump($m[1]);
