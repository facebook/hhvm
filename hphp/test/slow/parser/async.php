<?hh
abstract class C {
  async function f1() {}

  public async function f2() {}
  protected async function f3() {}
  private async function f4() {}
  static async function f5() {}
  final async function f7() {}

  async public function f8() {}
  async protected function f9() {}
  async private function f10() {}
  async static function f11() {}
  async final function f13() {}
}
echo "Done\n";

