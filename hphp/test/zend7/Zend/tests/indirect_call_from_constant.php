<?php

class Test
{
    public static function method()
    {
        echo "Method called!\n";
    }
}

['Test', 'method']();

'Test::method'();

(['Test', 'method'])();

('Test::method')();

?>
