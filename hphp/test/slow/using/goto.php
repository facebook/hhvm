<?php

require 'logger.inc';

function main() {
  echo "goto within using\n";
  using (new Logger()) {
    echo "About to goto\n";
    echo "Goto imminent\n";
    goto foo;
    echo "Shouldn't execute 1\n";
    foo:
    echo "At foo:\n";
  }
  echo "goto out of using\n";
  using (new Logger()) {
    echo "About to goto\n";
    goto foo2;
    echo "Shouldn't execute 2\n";
  }
  foo2:

  echo "Leaving main\n";
}
main();
