<?php

<<__EntryPoint>>
function main_subclass() {
$exec_class = ['Exception', 'RuntimeException', 'InvalidArgumentException',
               'BadMethodCallException', 'OutOfBoundsException',
               'DOMException', 'PDOException'];

foreach($exec_class as $e1) {
  foreach($exec_class as $e2) {
    echo $e1.' '.(is_subclass_of($e1,$e2)?'is':'is not').
         ' a subclass of '.$e2."\n";
  }
}
}
