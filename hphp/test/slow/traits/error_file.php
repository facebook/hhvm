<?hh

require "error_file.inc";

class X {
  use T;
}

(new X)->f();
(new X)->g();
