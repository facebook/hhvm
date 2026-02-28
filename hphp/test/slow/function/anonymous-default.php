<?hh

function f($val = (function() { return strtoupper("Foo"); })()) :mixed{
  echo "val = ";
  var_dump($val);
}


<<__EntryPoint>>
function main_anonymous_default() :mixed{
f();
}
