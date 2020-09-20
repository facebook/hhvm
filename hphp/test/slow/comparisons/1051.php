<?hh

function foo($p) {
  if ($p) {
    $a = 'foo';
  }
  if ('' < $a) {
    echo 'yes';
  }
 else {
    echo 'no';
  }
  if ($a > '') {
    echo 'yes';
  }
 else {
    echo 'no';
  }
}

<<__EntryPoint>>
function main_1051() {
foo(false);
}
