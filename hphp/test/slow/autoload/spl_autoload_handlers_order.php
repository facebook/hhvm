<?php

function a() {}
function b() {}
function c() {}
function d() {}

var_dump(spl_autoload_functions());
spl_autoload_register('a');
var_dump(spl_autoload_functions());
spl_autoload_register('a');
var_dump(spl_autoload_functions());
spl_autoload_register('b');
var_dump(spl_autoload_functions());
spl_autoload_register('c');
var_dump(spl_autoload_functions());
spl_autoload_register('d');
var_dump(spl_autoload_functions());
spl_autoload_register('a');
var_dump(spl_autoload_functions());

spl_autoload_unregister('a');
spl_autoload_unregister('b');
spl_autoload_unregister('c');
spl_autoload_unregister('d');
var_dump(spl_autoload_functions());

spl_autoload_register('a', true, true);
var_dump(spl_autoload_functions());
spl_autoload_register('b', true, true);
var_dump(spl_autoload_functions());
spl_autoload_register('a', true, true);
var_dump(spl_autoload_functions());
spl_autoload_register('c', true);
var_dump(spl_autoload_functions());
spl_autoload_unregister('a');
var_dump(spl_autoload_functions());
