//// a.php
<?hh

class A extends C {}

//// b.php
<?hh

class B extends C {}

//// c.php
<?hh

<<__Sealed(A::class)>>
class C {}

//////////////

//// c.php
<?hh

class C {}
