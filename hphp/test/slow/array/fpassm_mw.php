<?hh

function asd($x) :mixed{
  var_dump($x);
}

function foo($b) :mixed{
  asd($b[]);
}


<<__EntryPoint>>
function main_fpassm_mw() :mixed{
foo(vec[]);
}
