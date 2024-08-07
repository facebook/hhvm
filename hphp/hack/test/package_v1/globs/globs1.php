//// module_foo.php
<?hh
new module foo {}         // package pkg3
//// module_foobar.php
<?hh
new module foo.bar {}     // package pkg1
//// module_foobarbaz.php
<?hh
new module foo.bar.baz {} // package pkg2

//// foo.php
<?hh
module foo;
public class Foo {}

//// foobar.php
<?hh
module foo.bar;
public class FooBar {}

//// foobarbaz.php
<?hh
module foo.bar.baz;
public class FooBarBaz {}

//// test.php
<?hh
// default module
function test(): void {
   $x = new Foo();        // error: can't access pkg3
   $y = new FooBar();     // error: can't access pkg1
   $z = new FooBarBaz();  // error: can't access pkg2
}
