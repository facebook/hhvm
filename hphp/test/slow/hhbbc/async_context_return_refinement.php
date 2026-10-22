<?hh

final class AsyncContextReturnRefinement {
  public static async function aCaller(
  ): Awaitable<dict<string, dict<string, string>>> {
    return await self::zCallee(
      async () ==> true,
      'outer',
      'value',
    );
  }

  private static async function zCallee(
    (function(): Awaitable<bool>) $predicate,
    string $key,
    string $value,
  ): Awaitable<dict<string, dict<string, string>>> {
    $predicate();
    return dict[$key => dict['kind' => $value]];
  }
}

<<__EntryPoint>>
function main(): void {
  var_dump(HH\Asio\join(AsyncContextReturnRefinement::aCaller()));
}
