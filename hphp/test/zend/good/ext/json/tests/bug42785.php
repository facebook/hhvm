<?hh

class bar {}
<<__EntryPoint>>
function main_entry(): void {
  setlocale(LC_ALL, "de_DE", "de", "german", "ge", "de_DE.ISO8859-1", "ISO8859-1");

  $foo = vec[100.10,"bar"];
  var_dump(json_encode($foo));
  $bar1 = new bar;
  $bar1->a = 100.10;
  $bar1->b = "foo";
  var_dump(json_encode($bar1));
}
