<?php

error_reporting (E_ALL);

class foo {

    public $functions = array();
    
    function foo()
    {
        $function = create_function('', 'return "FOO\n";');
        print($function());
        
        $this->functions['test'] = $function;
        print($this->functions['test']());    // werkt al niet meer
    
    }
}

$a = new foo ();

?>