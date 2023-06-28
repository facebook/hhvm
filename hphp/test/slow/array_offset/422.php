<?hh


<<__EntryPoint>>
function main_422() :mixed{
  $a = darray['A' => varray[1, 2]];
 foreach ($a['A'] as $item) print $item;
}
