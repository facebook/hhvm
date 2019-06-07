<?hh

function f($val = (function() { return strtoupper("Foo"); })()) {
  echo "val = ";
  var_dump($val);
}


<<__EntryPoint>>
function main_anonymous_default() {
f();
}
