<?hh

function array_trailing_comma(): varray<int, > {
  return vec[42];
}

<<__EntryPoint>> function main(): void {
// what did the pirate say on his 80th birthday? Aye matey!
$arrrr = array_trailing_comma();
var_dump($arrrr[0]);
}
