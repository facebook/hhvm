////file1.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  const int P = 9;
}

////file2.hack
class B extends A {
  const arraykey P = 5;
}
