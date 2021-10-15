<?hh
<<file:__EnableUnstableFeatures('enum_class_label')>>

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
  var_dump(HH\facts_parse(NULL, vec[__FILE__], true, true));
  afunction();
}
