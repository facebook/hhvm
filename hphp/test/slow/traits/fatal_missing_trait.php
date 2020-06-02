<?hh

<<__EntryPoint>>
function entrypoint_fatal_missing_trait(): void {

  if (isset($g)) {
    include 'fatal_missing_trait.inc';
  }

  include 'fatal_missing_trait-class.inc';

  var_dump(new C());
}
