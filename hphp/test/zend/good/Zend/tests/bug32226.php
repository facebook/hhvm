<?php

class A
{
        public function A()
        {
                set_exception_handler(array($this, 'EH'));

                throw new Exception();
        }

        public function EH()
        {
                restore_exception_handler();

                throw new Exception();
        }
}

try
{
$a = new A();
}
catch(Exception $e)
{
	echo "Caught\n";
}

?>
===DONE===