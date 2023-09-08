<?hh

enum class E: int {
  int A = 40;
  int B = 42;
}

<<__EntryPoint>>
function main() {
  echo "SAME\n";
  var_dump(E#A === E#A);
  var_dump(E#A !== E#A);
  var_dump(E#A == E#A);
  var_dump(E#A != E#A);

  echo "\nDIFFERENT\n";
  var_dump(E#A === E#B);
  var_dump(E#A !== E#B);
  var_dump(E#A == E#B);
  var_dump(E#A != E#B);

  echo "\nOPPOSITE\n";
  var_dump(E#B === E#A);
  var_dump(E#B !== E#A);
  var_dump(E#B == E#A);
  var_dump(E#B != E#A);

  echo "\nFAIL\n";
  var_dump(E#A > E#A);
}
