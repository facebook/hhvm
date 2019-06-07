<?hh


function foo($x) {
  bar(

    foo($x)

  );
}

function bar($x) {
  var_dump(__METHOD__);
}


<<__EntryPoint>>
function main_nested() {
foo(0);
}
