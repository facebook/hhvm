<?php

class Evil {
  public function __destruct() {
    echo "in __destruct(): ";
    var_dump($this);
    try {
      dumpCurrent();
    } catch (Exception $e) {
      printf("Caught: %s\n", $e->getMessage());
    }
  }
}
function dumpCurrent() {
  echo "current: ";
  var_dump($GLOBALS['cont']->current());
  if (isset($GLOBALS['gonext'])) {
    echo "iter from destructor\n";
    $GLOBALS['cont']->next();
  }
}
function gen() {
  echo "gen 1\n";
  yield new Evil;
  echo "gen 2\n";
  yield null;
  echo "gen 3\n";
  yield new Evil;
  echo "gen 4\n";
  yield new Evil;
  echo "gen 5\n";
}
function main() {
  $GLOBALS['cont'] = $c = gen();
  echo "iter 1\n";
  $c->rewind();

  echo "iter 2\n";
  $c->send(new Evil);
  $GLOBALS['gonext'] = true;
  echo "iter 3\n";
  $c->next();
  echo "iter 4\n";
  $c->send(null);
  echo "iter 5\n";
  $c->send(null);
  echo "Finished!\n";
}
main();
echo "Returned from main safely\n";
