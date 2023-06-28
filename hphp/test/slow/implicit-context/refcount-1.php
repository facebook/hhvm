<?hh

<<__EntryPoint>>
function main() :mixed{
  $cls = HH\ImplicitContext\soft_run_with(
    () ==> HH\ImplicitContext\embed_implicit_context_state_in_closure(
      () ==> HH\ImplicitContext\_Private\get_whole_implicit_context(),
    ),
    'abc',
  );
  $cls();
  echo "Done\n";
}
