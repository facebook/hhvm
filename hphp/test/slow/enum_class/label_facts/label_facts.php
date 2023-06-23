<?hh

enum class Something: mixed {
  mixed a = 5;
}

function afunction(): void {
  my_function(#a);
}

function my_function(
  HH\EnumClass\Label<Something, mixed> $param,
): void {
  echo "Hello, world.\n";
}

<<__EntryPoint>>
function main_hashbang() {
  var_dump(HH\Facts\path_to_functions(__FILE__));
  var_dump(HH\Facts\path_to_types(__FILE__));
  var_dump(HH\Facts\kind("Something"));
  var_dump(HH\Facts\supertypes("Something"));
  var_dump(HH\Facts\is_abstract("Something"));
  var_dump(HH\Facts\is_final("Something"));
  afunction();
}
