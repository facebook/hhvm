<?hh
<<file:__EnableUnstableFeatures('typed_local_variables')>>

<<__EntryPoint>>
function main(): void {
  let $x: string;
  try {
    $x = 1;
  }
  catch (Throwable $e) {
    print("Caught Throwable\n");
  }
}
