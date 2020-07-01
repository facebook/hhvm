<?hh

<<__EntryPoint>>
function main(): void {
  $dict = dict[];
  $dict[:@my_message] = "I am an atom !";

  foreach ($dict as $key => $value) {
    echo "key: ".$key."\n";
    echo "value: ".$value."\n";
  }

  $shape = shape("toto" => 42);
  echo $shape[:@toto]."\n";
}
