<?php

function getVar() { return "var"; }


<<__EntryPoint>>
function main_interpolated_function_call() {
$var = "test";

echo "${getVar()}\n";
}
