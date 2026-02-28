//// a.php
<?hh

case type MyArraykey = int | string;

//// b.php
<?hh

function myfoo(MyArraykey $x): arraykey {
  if ($x is arraykey) {
    return $x;
  }
  throw new Exception();
}
