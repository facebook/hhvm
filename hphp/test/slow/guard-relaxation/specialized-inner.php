<?hh


function callee(inout HH\Map $c) :mixed{

}

function main() :mixed{
  $c = HH\Map {};
  callee(inout $c);
  var_dump($c);
}


<<__EntryPoint>>
function main_specialized_inner() :mixed{
main();
}
