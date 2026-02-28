<?hh

<<__EntryPoint>>
function main_apache_notes() :mixed{
  apache_notes(dict[
    "blarb" => "foo",
    "foo" => "bar",
  ]);
  if (apache_note("blarb", "smurf") === "foo") {
    echo "ok\n";
  }
  if (apache_note("blarb") === "smurf") {
    echo "ok\n";
  }
  if (apache_note('foo') === 'bar') {
    echo "ok\n";
  }
  try {
    apache_notes(dict[42 => null]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
  try {
    apache_notes(dict["42" => 42]);
  } catch (Exception $e) {
    var_dump($e->getMessage());
  }
}
