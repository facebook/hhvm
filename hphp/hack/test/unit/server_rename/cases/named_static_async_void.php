<?hh
//
<<file: __EnableUnstableFeatures('named_parameters')>>
class C {
  public static async function original(named int $b): Awaitable<void> {}
}
