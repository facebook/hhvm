<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function identity($x) { return $x; }

$x = fb_call_user_func_safe('identity', 'abc');
var_dump($x);
var_dump(is_varray($x));
var_dump(is_darray($x));
