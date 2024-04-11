//// a.php
<?hh

enum class A : mixed {}

//// b.php
<?hh

enum class B : mixed extends A {}

//// c.php
<?hh

enum class C : mixed {
  int X = 1;
}

//// d.php
<?hh

enum class D : mixed extends C {}

//// e.php
<?hh

enum class E : mixed extends B, D {}

////////////////////////

//// a.php
<?hh

enum class A : mixed {
  int X = 0;
}
