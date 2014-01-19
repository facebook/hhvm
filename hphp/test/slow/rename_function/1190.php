<?php

function test1() {
 print __FUNCTION__;
}
 function test2() {
 print __FUNCTION__;
}
 fb_rename_function('test2', 'test3');
fb_rename_function('test1', 'test2');
 teSt2();
fb_rename_function('test2', 'test3');
 teSt2();
