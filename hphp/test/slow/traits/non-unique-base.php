<?hh

<<__EntryPoint>>
function entrypoint_nonuniquebase(): void {

  if (getenv("NOFOO")) {
    include 'non-unique-base-1.inc';
  } else {
    include 'non-unique-base-2.inc';
  }
  include 'non-unique-base-main.inc';

  $obj = new C;
  $obj->main();
}
