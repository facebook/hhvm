<?hh


function callee(inout HH\Map $c) {

}

function main() {
  $c = HH\Map {};
  callee(inout $c);
  var_dump($c);
}


<<__EntryPoint>>
function main_specialized_inner() {
main();
}
