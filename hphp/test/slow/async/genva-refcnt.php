<?hh // decl

class Marker {
}

async function foo() {
  return new Marker();
}

async function bar() {
  var_dump(objprof_get_data());
  echo "genva 1\n";
  concurrent {
    $a = await foo();
    $b = await foo();
  }
  echo "unset a\n";
  unset($a);
  echo "unset b\n";
  unset($b);
  echo "genva 2\n";
  concurrent {
    await foo();
    await foo();
  }
  echo "genva 3\n";
  concurrent {
    await foo();
    await foo();
  }
  echo "done\n";
  var_dump(objprof_get_data());
}


<<__EntryPoint>>
function main_genva_refcnt() {
\HH\Asio\join(bar());
echo "exit\n";
}
