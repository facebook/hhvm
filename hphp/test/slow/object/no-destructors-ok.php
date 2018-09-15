<?hh

class HaveDestructor {
  function __destruct() {}
}

class NoDestructor {
}


<<__EntryPoint>>
function main_no_destructors_ok() {
new NoDestructor();
echo "Should get here\n";
}
