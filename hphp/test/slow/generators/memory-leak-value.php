<?hh

class C {}

function generator($n) :AsyncGenerator<mixed,mixed,void>{
  --$n;
  while ($n > 0) {
    yield new C();
    --$n;
  }
}

<<__EntryPoint>>
function main() :mixed{
  ini_set('memory_limit', 30000);

  foreach (generator(100000) as $_) {
  }

  echo "OK\n";
}
