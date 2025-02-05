//// a.php
<?hh

final class A {

  const string B = 'B';
  const string C = 'C';
}

//// b.php
<?hh

type S = shape(
  A::B => int,
  A::C => int,
  ...
);
