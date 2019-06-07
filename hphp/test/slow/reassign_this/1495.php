<?hh
<<__EntryPoint>> function main() {
$myarray = array(1 => 2, 2 => 3);
 foreach ($myarray as $this => $wat) {
  echo "You should not see this";
 }
}
