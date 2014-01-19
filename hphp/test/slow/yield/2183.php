<?php

class Evil {
  public function __destruct() {
    echo "in __destruct()\n";
    try {
      dumpCurrent();
    }
 catch (Exception $e) {
      printf("Caught: %s\n", $e->getMessage());
    }
  }
}
function dumpCurrent() {
  var_dump($GLOBALS['cont']->current());
  if (isset($GLOBALS['gonext'])) {
    $GLOBALS['cont']->next();
  }
}
function gen() {
  yield new Evil;
  yield null;
  yield new Evil;
  yield new Evil;
}
function main() {
  $GLOBALS['cont'] = $c = gen();
  $c->next();

  $c->send(new Evil);
  $GLOBALS['gonext'] = true;
  $c->next();
  $c->send(null);
  $c->send(null);
  echo "Finished!\n";
}
main();
echo "Returned from main safely\n";
