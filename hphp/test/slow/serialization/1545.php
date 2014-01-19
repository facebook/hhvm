<?php

var_dump(unserialize('a:1:{s:1:"1";s:3:"foo";}'));
var_dump(unserialize('a:1:{d:1;s:3:"foo";}'));
var_dump(unserialize('a:1:{a:1:{i:0;i:1;}}'));
