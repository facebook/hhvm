<?hh

function foo() {
  if (!interface_exists('MyInterface')) {
    include '1218.inc';
    echo 'no';
  }
 else {
    echo 'yes';
  }
}

<<__EntryPoint>>
function main_1218() {
foo();
foo();
}
