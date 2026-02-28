<?hh
function f(int $i) :mixed{
  var_dump($i);
}
function g(string $s) :mixed{
  var_dump($s);
}


<<__EntryPoint>>
function main_2189() :mixed{
g(DATE_RFC850);
f(count(vec[]));
}
