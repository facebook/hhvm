<?hh

class HaveDestructor {
  function __destruct() {}
}

class NoDestructor {
}

new NoDestructor();
echo "Should get here\n";
