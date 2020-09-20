<?hh
<<__EntryPoint>> function main(): void {
invariant_callback_register(function(...$args) {
  var_dump($args);
});
invariant(false, "a", "b");
}
