<?php
<<__EntryPoint>> function main() {
var_dump(realpath('.') == realpath(getcwd()));
chdir('..');
var_dump(realpath('.') == realpath(getcwd()));
}
