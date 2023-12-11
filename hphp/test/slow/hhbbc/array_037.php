<?hh

function aa() :mixed{ return 'a'; }
function heh() :mixed{ return dict['a' => aa()]; }
function x() :mixed{
  $x = heh();
  $r = $x[''] = 2;
  var_dump($r);
  var_dump($x);
}

<<__EntryPoint>>
function main_array_037() :mixed{
x();
}
