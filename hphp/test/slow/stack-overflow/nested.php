<?hh


function foo($x) :mixed{
  bar(

    foo($x)

  );
}

function bar($x) :mixed{
  var_dump(__METHOD__);
}


<<__EntryPoint>>
function main_nested() :mixed{
foo(0);
}
