<?hh


function main($a, $i) {
  var_dump(isset($a[1 << 32]));
  var_dump(isset($a[$i]));
}
<<__EntryPoint>> function main_entry(): void {
main(varray[1, 2, 3, 4], 1 << 32 + 1);
}
