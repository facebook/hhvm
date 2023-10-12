////file1.php
<?hh // strict

class C1 {
  const type T = C2::T;
}

class C2 {
  const type T = C1::T;
}

class C3 {
  const type T = C1::T;
}

////file2.php
<?hh
function foo(C3::T $x):void { }
