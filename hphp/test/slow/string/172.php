<?php

var_dump(strncmp('foo', 'foo', -100));
var_dump(strncasecmp('foo', 'foo', -100));
var_dump(substr_compare('foo', 'foo', 0, -100, true));
var_dump(substr_compare('foo', 'bar', 3));
var_dump(substr_compare('foo', 'bar', -3));
