<?php
 
trait myTrait {
     public function run() {}
}

class myClass {
     use myTrait {
         MyTrait::run as private;
     }
}
$class = new \ReflectionClass('myClass');
var_dump($class->getTraitAliases());

?>