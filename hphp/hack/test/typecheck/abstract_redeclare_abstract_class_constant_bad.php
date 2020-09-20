////file1.php
<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class A {
  abstract const int P;
}

////file2.php
<?hh

abstract class B extends A {
  abstract const arraykey P;
}
