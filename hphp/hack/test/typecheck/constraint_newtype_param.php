//// file1.php
<?hh
class A<T as bool> {}
newtype B<T as int> = A<T>;

//// file2.php
<?hh
function foo(B<string> $_): void {}
