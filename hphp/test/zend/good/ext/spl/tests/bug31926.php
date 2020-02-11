<?hh
<<__EntryPoint>> function main(): void {
$array = darray[0 => varray['world']];

$it = new RecursiveIteratorIterator(new RecursiveArrayIterator($array));
foreach($it as $key => $val) {
   var_dump($key, $val);
}
}
