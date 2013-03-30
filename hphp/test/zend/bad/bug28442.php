<?php

class ClassA
{
   static $prop;
}

class ClassB extends ClassA
{
   static $prop;
}

class ClassC extends ClassB
{
}

echo "===INIT===\n";
ClassA::$prop = 'A';
ClassB::$prop = 'B';
ClassC::$prop = 'C';
var_dump(ClassA::$prop);
var_dump(ClassB::$prop);
var_dump(ClassC::$prop);

echo "===SetA===\n";
ClassA::$prop = 'A2';
var_dump(ClassA::$prop);
var_dump(ClassB::$prop);
var_dump(ClassC::$prop);

echo "===SetB===\n";
ClassB::$prop = 'B2';
var_dump(ClassA::$prop);
var_dump(ClassB::$prop);
var_dump(ClassC::$prop);

echo "===SetC===\n";
ClassC::$prop = 'C2';
var_dump(ClassA::$prop);
var_dump(ClassB::$prop);
var_dump(ClassC::$prop);

?>
===DONE===