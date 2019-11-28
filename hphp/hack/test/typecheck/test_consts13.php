////file1.php
<?hh // strict

abstract class C1 {
  abstract const int X;
}
////file2.php
<?hh
class C2 extends C1 {
  const string X = 'a'; // error: unexpected type
}
