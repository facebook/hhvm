<?php

include "queryable.inc";

function into() { return "ok\n"; }

$students = new Queryable();

$q = from $student in $students
     group $student by $student->Year into $years
       from $student in $years
       group $student by $student->LastName;

foreach ($q as $e) {
  echo $e."\n";
}

echo into();
