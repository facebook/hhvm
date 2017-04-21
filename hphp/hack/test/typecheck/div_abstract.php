////file1.php
<?hh // strict

newtype MyFloat as float = float;

////file2.php
<?hh // strict
  function MyDiv(MyFloat $x, MyFloat $y): float {
    return $x / $y;
    }

function MyDivAlt<Tx as float, Ty as float>(Tx $x, Ty $y): float {
  return $x / $y;
}
