<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>
class C {
  public async function original(named int $b): Awaitable<int> {
    return $b;
  }
}
