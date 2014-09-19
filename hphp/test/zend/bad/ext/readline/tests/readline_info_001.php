<?php

var_dump(readline_info());
var_dump(readline_info(1));
var_dump(readline_info(1,1));
var_dump(readline_info('line_buffer'));
var_dump(readline_info('readline_name'));
var_dump(readline_info('readline_name', 1));
var_dump(readline_info('readline_name'));
var_dump(readline_info('attempted_completion_over',1));
var_dump(readline_info('attempted_completion_over'));

?>
