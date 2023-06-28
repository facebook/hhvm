<?hh

<<__EntryPoint>>
async function main() :Awaitable<mixed>{
  await HH\ImplicitContext\soft_run_with_async(
    async () ==> {
      $hash = HH\ImplicitContext\_Private\get_implicit_context_memo_key();
      echo "Hash: ".quoted_printable_encode($hash)."\n";
    },
    __FUNCTION__,
  );
}
