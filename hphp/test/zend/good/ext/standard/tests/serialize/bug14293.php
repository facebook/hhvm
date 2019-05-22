<?php
class t
{
    function __construct()
    {
        $this->a = 'hello';
    }

    function __sleep()
    {
        echo "__sleep called\n";
        return array('a','b');
    }
}
<<__EntryPoint>> function main() {
$t = new t();
$data = serialize($t);
echo "$data\n";
$t = unserialize($data);
var_dump($t);
}
