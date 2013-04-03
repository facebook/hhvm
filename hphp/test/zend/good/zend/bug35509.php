<?php
class mytest
{
  const classConstant = '01';

  private $classArray = array( mytest::classConstant => 'value' );

  public function __construct()
  {
    print_r($this->classArray);
  }
}

$classtest = new mytest();

define( "normalConstant", '01' );
$normalArray = array( normalConstant => 'value' );
print_r($normalArray);
?>