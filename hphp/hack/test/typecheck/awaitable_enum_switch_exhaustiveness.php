<?hh

enum SwitchDemoEnum: string {
  A = 'A';
  B = 'B';
}

class C {
  public static async function genEnum(): Awaitable<SwitchDemoEnum> {
    return SwitchDemoEnum::A;
  }

  public static async function genTest(): Awaitable<void> {
    $enum = await self::genEnum();
    switch ($enum) {
      case SwitchDemoEnum::A:
        break;
    }
  }
}
