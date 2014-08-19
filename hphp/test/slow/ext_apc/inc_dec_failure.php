<?php
apc_delete('test');
var_dump(apc_inc('test'));
var_dump(apc_dec('test'));

apc_store('x', 'x');

var_dump(apc_inc('x'));
var_dump(apc_dec('x'));

apc_store('numeric_str', '1');

var_dump(apc_inc('numeric_str'));
var_dump(apc_dec('numeric_str'));
