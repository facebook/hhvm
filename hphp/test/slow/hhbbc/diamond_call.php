<?hh

abstract class something { abstract function go():mixed; }

class asd extends something { function go() :mixed{ echo "asd\n"; } }
class bsd extends something { function go() :mixed{ echo "bsd\n"; } }

function call_something($x) :mixed{ for ($i = 0; $i < 10; ++$i) { mt_rand(); } }
function call_something_else($x) :mixed{ for ($i = 0; $i < 10; ++$i) {mt_rand(); } }

function diamond(something $state) :mixed{
  if ($state is asd) {
    call_something($state);
    for ($i = 0; $i < 10; ++$i) { mt_rand(); }
  } else if ($state is bsd) {
    call_something_else($state);
  }

  $state->go();
  echo "done\n";
}


<<__EntryPoint>>
function main_diamond_call() :mixed{
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
}
