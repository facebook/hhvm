<?hh

require "error_file.inc2";

trait T {
  use U;
  function g() { echo $x; }
}
