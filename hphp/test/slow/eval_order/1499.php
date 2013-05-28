<?php

set_error_handler('h');
foo(var_dump('123'));
var_dump('end');
function h() {
 var_dump('errored');
}
