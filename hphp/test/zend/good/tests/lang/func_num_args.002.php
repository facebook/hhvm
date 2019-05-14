<?php

function foo($a)
{
    var_dump(func_num_args());
}
<<__EntryPoint>> function main() {
foo(1, 2, 3);
}
