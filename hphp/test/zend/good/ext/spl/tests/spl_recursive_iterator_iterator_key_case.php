<?hh <<__EntryPoint>> function main(): void {
$ar = darray["one"=>1, "two"=>2, "three"=>darray["four"=>4, "five"=>5, "six"=>darray["seven"=>7]], "eight"=>8, -100 => 10, NULL => "null"];
$it = new RecursiveArrayIterator($ar);
$it = new RecursiveIteratorIterator($it);
foreach($it as $k=>$v)
{
  echo "$k=>$v\n";
  var_dump($k);
}
}
