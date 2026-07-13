<?hh

function preinc(inout $x):mixed{ ++$x; return $x; }

function test() :mixed{
  $a = 2;
  switch ($a) {
    case preinc(inout $a): var_dump('ok');
 break;
    case 2: var_dump('broken');
 break;
    case 3: var_dump('really broken');
 break;
    default: var_dump('fail');
 break;
  }
}

<<__EntryPoint>>
function main_1753() :mixed{
$a = 2;
switch ($a) {
  case preinc(inout $a): var_dump('ok');
 break;
  case 2: var_dump('broken');
 break;
  case 3: var_dump('really broken');
 break;
  default: var_dump('fail');
 break;
}
test();
}
