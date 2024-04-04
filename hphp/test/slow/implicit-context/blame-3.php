<?hh

<<__Memoize>>
function memo(): void {}

<<__EntryPoint>>
function main(): void {
  set_error_handler((...$_args) ==> {
    HH\ImplicitContext\soft_run_with(
      memo<>,
      'abc',
    );
    var_dump(HH\deferred_errors());
    return true;
  });
  trigger_error('yo');
}
