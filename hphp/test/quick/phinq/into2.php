<?php

include "queryable.inc";

class foo { public $students; public $start; }

$f = new foo();
$f->students = new Queryable();
$f->start = 3;

$q = from $student in $f->students
     group $student by $student->Year into $years
       from $student in $years
       where $student->Year == $f->start
       group $student by $student->LastName;

foreach ($q as $e) {
  print_result($e);
}
