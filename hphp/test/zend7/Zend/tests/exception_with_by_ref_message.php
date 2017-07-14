<?php

class MyException extends Exception
{
    public function __construct(&$msg) {
        $this->message =& $msg;
    }
}

$msg = 'Message';
throw new MyException($msg);

?>
