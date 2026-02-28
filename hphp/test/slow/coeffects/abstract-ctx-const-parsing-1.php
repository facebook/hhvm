<?hh

abstract class A {
  const ctx C1 = [];
  abstract const ctx C2;
  abstract const ctx C3 = [];
  const ctx C4 = [random_coeffect_who_am_i];
}

<<__EntryPoint>>
function main() :mixed{
  echo "ok\n";
}
