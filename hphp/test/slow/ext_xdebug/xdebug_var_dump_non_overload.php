<?php
  class TimeStuff {
    private $timestamp;
    function TimeStuff($ts = null)
    {
      $this->timestamp = $ts === null ? time() : $ts;
    }
  }
$ts1 = new TimeStuff(1092515106);
var_dump($ts1);
ini_set('xdebug.overload_var_dump', 1);
var_dump($ts1);
echo "\n";
ini_set('xdebug.overload_var_dump', 0);
var_dump($ts1);
