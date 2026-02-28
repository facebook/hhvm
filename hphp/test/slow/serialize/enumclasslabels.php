<?hh

enum class E : int {
  int A = 0;
  int B = 1;
}

<<__EntryPoint>> function main(): void {
  echo serialize(E#A) . "\n";
  echo serialize(E#B) . "\n";

  var_dump(unserialize(serialize(E#A)));
  echo E::valueOf(unserialize(serialize(E#A))) . "\n";
}
