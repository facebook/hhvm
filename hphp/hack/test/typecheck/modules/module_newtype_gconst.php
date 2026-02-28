//// module.php
<?hh

new module foo {}

//// decl.php
<?hh

module foo;
module newtype FooType as string = string;

//// use.php
<?hh

module foo;
namespace Foo;

const \FooType foo_const = 'foo';
