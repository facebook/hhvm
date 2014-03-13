<?hh

abstract class something { abstract function go(); }

class asd extends something { function go() { echo "asd\n"; } }
class bsd extends something { function go() { echo "bsd\n"; } }

function call_something($x) { for ($i = 0; $i < 10; ++$i) { mt_rand(); } }
function call_something_else($x) { for ($i = 0; $i < 10; ++$i) {mt_rand(); } }

function diamond(something $state) {
  if ($state instanceof asd) {
    call_something($state);
    for ($i = 0; $i < 10; ++$i) { mt_rand(); }
  } else if ($state instanceof bsd) {
    call_something_else($state);
  }

  $state->go();
  echo "done\n";
}

diamond(new bsd);
diamond(new bsd);
diamond(new bsd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
diamond(new asd);
for ($i = 0; $i < 1000; ++$i) { diamond(new asd); }
