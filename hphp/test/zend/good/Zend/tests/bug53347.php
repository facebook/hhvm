<?php class ezcConsoleOutput
{
    protected static $color = array( 'gray' => 30 );

    public static function isValidFormatCode( $type, $key )
    {
        return isset( self::${$type}[$key] );
    }
}

var_dump( ezcConsoleOutput::isValidFormatCode( 'color', 'gray' ) );
?>