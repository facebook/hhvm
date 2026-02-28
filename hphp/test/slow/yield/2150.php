<?hh

function nums() :AsyncGenerator<mixed,mixed,void>{
 $i = 0;
 while (true) {
  switch ($i) {
    case 0: yield $i;
    $i = 1;
    case 999: yield $i;
    break;
    $i = -1;
    case 1: $i = 2;
    yield $i;
    yield break;
  }
 }
}


 <<__EntryPoint>>
function main_2150() :mixed{
foreach (nums() as $num) {
 var_dump($num);
}
}
