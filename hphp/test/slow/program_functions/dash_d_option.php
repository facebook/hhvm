<?php


<<__EntryPoint>>
function main_dash_d_option() {
var_dump(ini_get("hhvm.log.level"));
var_dump(ini_get("this.should.not.work"));
}
