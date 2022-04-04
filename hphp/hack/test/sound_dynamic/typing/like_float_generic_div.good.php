<?hh
function MyDivAlt<Tx  as float, Ty as float>(~Tx $x, ~Ty $y): ~float {
  $z = $x / $y;
  return $z;
}
function MyAddAlt<Tx  as float, Ty as float>(~Tx $x, ~Ty $y): ~float {
  $z = $x + $y;
  return $z;
}
