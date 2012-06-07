<?php

// NB: The names need to be all-lowercase to ensure that "someexception" has a
// lower litstr id than "other" (both original and lowercase versions of the
// name get put in the litstr table). This test relies on someexception having a
// lower litstr id to reproduce a specific bug.

class someexception extends Exception {}
class other extends someexception {}

function main() {
  try {
    throw new other;
  } catch (other $e) {
    echo "win\n";
  } catch (someexception $e) {
    echo "fail\n";
  }
}
main();
