//// a.php
<?hh

enum class A : mixed {}

//// b.php
<?hh

enum class B : mixed extends A {}

//// c.php
<?hh

enum class C : mixed extends B {
   string C = '';
}

////////////////////////

//// a.php
<?hh

enum class A : mixed {
  string C = 'oops';
}
