<?hh

<<__EntryPoint>>
function main_1() :mixed{
apache_note("blarb", "foo");
if (apache_note("blarb", "smurf") === "foo") {
  echo "ok\n";
}
if (apache_note("blarb") === "smurf") {
  echo "ok\n";
}
}
