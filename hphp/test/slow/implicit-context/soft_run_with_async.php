<?hh

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  await HH\ImplicitContext\soft_run_with_async(
    async () ==> {
      $hash = HH\ImplicitContext\_Private\get_implicit_context_debug_info();
      $hash = HH\Lib\Str\join($hash, ", ");
      echo "Hash:\n";
      echo var_dump($hash);
    },
    __FUNCTION__,
  );
}
