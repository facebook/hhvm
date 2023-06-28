<?hh
abstract class C {
  async function f1() :Awaitable<mixed>{}

  public async function f2() :Awaitable<mixed>{}
  protected async function f3() :Awaitable<mixed>{}
  private async function f4() :Awaitable<mixed>{}
  static async function f5() :Awaitable<mixed>{}
  final async function f7() :Awaitable<mixed>{}

  async public function f8() :Awaitable<mixed>{}
  async protected function f9() :Awaitable<mixed>{}
  async private function f10() :Awaitable<mixed>{}
  async static function f11() :Awaitable<mixed>{}
  async final function f13() :Awaitable<mixed>{}
}

<<__EntryPoint>>
function main_async() :mixed{
echo "Done\n";
}
