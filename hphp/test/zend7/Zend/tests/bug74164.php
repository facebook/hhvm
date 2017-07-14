<?php

namespace Foo;

set_error_handler(function ($type, $msg) {
	throw new \Exception($msg);
});

call_user_func(function (array &$ref) {var_dump("xxx");}, 'not_an_array_variable');
?>
