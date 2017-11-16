<?hh

class HaveDestructor {
  function __destruct() {}
}

echo "Should get here\n";
new HaveDestructor();
echo "Should not get here\n";
