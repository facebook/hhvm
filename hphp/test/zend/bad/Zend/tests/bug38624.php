<?php

class impl
{
    public function __construct()
    {
       $this->counter++;
    }
    public function __set( $name, $value )
    {
        throw new Exception( "doesn't work" );
    }

    public function __get( $name )
    {
        throw new Exception( "doesn't work" );
    }

}

$impl = new impl();

echo "Done\n";
?>