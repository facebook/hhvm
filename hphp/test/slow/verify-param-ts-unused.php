<?hh

interface InterfaceFoo {
  public function genVisit<reify T>(
        ?T $plain_value
  ): Awaitable<void>;
}

final class Visitor implements InterfaceFoo {
  protected int $counter = 0;

  public async function genVisit<reify T>(
    ?T $plain_value
  ): Awaitable<void> {
    $this->counter++;
  }
}

async function bar(InterfaceFoo $o) :Awaitable<mixed>{
  $i = rand(1, 45);
  await $o->genVisit<int>($i);
}

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  $visitor = new Visitor();
  await bar($visitor);
  echo "done.\n";
}
