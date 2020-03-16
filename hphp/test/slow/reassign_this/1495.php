<?hh
<<__EntryPoint>> function main(): void {
$myarray = darray[1 => 2, 2 => 3];
 foreach ($myarray as $this => $wat) {
  echo "You should not see this";
 }
}
