<?php
trait abc
{
  static function def()
  {
    echo self::class, "\n";
    echo __CLASS__, "\n";
  }
}

class ghi
{
  use abc;
}

ghi::def();
?>
