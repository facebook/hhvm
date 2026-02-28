//// modules.php
<?hh
new module foo {}

//// test.php
<?hh
module foo;
internal class Sealed implements HH\ClassAttribute {}


//// test2.php
<?hh
<<Sealed>>
trait TBar {}
