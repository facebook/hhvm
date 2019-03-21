<?hh

invariant_callback_register(function(...$args) {
  var_dump($args);
});
invariant(false, "a", "b");
