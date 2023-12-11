<?hh

function show($x) :mixed{
  if ($x is (int, string)) {
    var_dump($x[0]);
    var_dump($x[1]);
  }
}

<<__EntryPoint>>
function main() :mixed{
  show(vec[17, 34]);
  show(vec[17, 'ace']);
}
