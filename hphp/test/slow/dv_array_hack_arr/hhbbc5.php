<?hh

function show($x) :mixed{
  if ($x is (int, string)) {
    var_dump($x[0]);
    var_dump($x[1]);
  }
}

<<__EntryPoint>>
function main() :mixed{
  show(varray[17, 34]);
  show(varray[17, 'ace']);
}
