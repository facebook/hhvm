//// base-a.php
<?hh
class A<T> {}
//// base-b.php
<?hh
class B<T> extends A<T> {}
//// base-use-b.php
<?hh
function use_b(): void {
  $_ = new B<int>();
}

//// changed-a.php
<?hh
class A<TU> {}
//// changed-b.php
<?hh
class B<T> extends A<T> {}
//// changed-use-b.php
<?hh
function use_b(): void {
  $_ = new B<int>();
}
