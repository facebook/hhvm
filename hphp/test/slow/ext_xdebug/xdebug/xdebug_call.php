<?php
class a {
  public function __construct( $var )
  {
    echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
    c( $var + 1);
  }

  public function aa( $var )
  {
    echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
    a::b( $var + 1 );
  }

  static public function b( $var )
  {
    echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
    c( $var + 1);
  }
}

function c( $var )
{
  echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
  d( $var + 1 );
}

function d( $var )
{
  echo $var, ': ', xdebug_call_class(), '>', xdebug_call_function(), ' @ ', xdebug_call_file(), ':', xdebug_call_line(), "\n";
}

d( 1 );
echo "\n";
c( 1 );
echo "\n";
a::b( 1 );
echo "\n";
$a = new a( 1 );
echo "\n";
$a->aa( 1 );
