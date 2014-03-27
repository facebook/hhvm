<?hh

invariant_callback_register(function() {
  var_dump(func_get_args());
});
invariant(false, "a", "b");
