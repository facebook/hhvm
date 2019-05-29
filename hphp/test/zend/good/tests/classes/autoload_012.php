<?php
function __autoload($name)
{
    echo "In autoload: ";
    var_dump($name);
}
<<__EntryPoint>> function main() {
call_user_func("UndefC::test");
}
