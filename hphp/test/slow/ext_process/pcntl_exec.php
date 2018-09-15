<?php


<<__EntryPoint>>
function main_pcntl_exec() {
pcntl_exec("/bin/sh",
             array(__DIR__."/test_pcntl_exec.sh"),
             array("name" => "value"));
}
