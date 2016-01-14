<?hh

function array_trailing_comma(): array<int, > {
  return [42];
}

// what did the pirate say on his 80th birthday? Aye matey!
$arrrr = array_trailing_comma();
var_dump($arrrr[0]);
