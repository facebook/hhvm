<?php


function one() { echo 'one';}
fb_renamed_functions(array('one', 'three'));
var_dump(fb_rename_function('one', 'two'));
