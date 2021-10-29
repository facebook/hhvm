<?hh

function defaults() {}

<<__EntryPoint>>
function pure()[] {
  for ($i = 0; $i < 10; $i++) {
    defaults();
  }
  echo "done\n";
}
