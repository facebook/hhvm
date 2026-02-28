//// modules.php
<?hh


new module cookies {}

//// test.php
<?hh

module cookies;


internal class A {}

class B extends A{}
