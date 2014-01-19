<?php

function err($code,$msg) {
 var_dump($code,$msg);
 }
set_error_handler('err');
function test1() {
}
function test2() {
}
fb_rename_function('test1', 'test3');
fb_rename_function('test2', 'test1');
fb_rename_function('test1', 'test2');
fb_rename_function('test3', 'test1');
