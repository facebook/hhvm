<?php

error_reporting (E_ALL);

class foo {

    public $functions = array();

    function __construct()
    {
        $function = () ==> "FOO\n";
        print($function());

        $this->functions['test'] = $function;
        print($this->functions['test']());    // werkt al niet meer

    }
}

$a = new foo ();

