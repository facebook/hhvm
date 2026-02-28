<?hh

function defaults() :mixed{}

<<__EntryPoint>>
function pure()[] :mixed{
  for ($i = 0; $i < 10; $i++) {
    defaults();
  }
  echo "done\n";
}
