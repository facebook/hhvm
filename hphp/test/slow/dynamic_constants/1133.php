<?php


<<__EntryPoint>>
function main_1133() {
define('FOO', BAR);
 define('BAR', FOO);
 echo FOO;
 echo BAR;
}
