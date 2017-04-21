<?php
class MyException extends Exception
{
  protected $file = 7;
  protected $line = 'abc';
}

$exception = new MyException('Error', 1234);
echo "DONE\n";
