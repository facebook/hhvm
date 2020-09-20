<?hh

namespace vec {
const STUFF = 12;
function vec() { return 0; }
function foo() { return 1; }
}

namespace alpha {
const STUFF = 12;
function vec() { return 0; }
function foo() { return 1; }
}

namespace beta\vec {
const STUFF = 12;
function vec() { return 0; }
function foo() { return 1; }
}

namespace {
function vecnest(vec<vec<int>> $foo) {}
<<__EntryPoint>>
function main() {
  var_dump(\vec\vec());
  var_dump(\vec\foo());
  var_dump(\vec\STUFF);

  var_dump(\alpha\vec());
  var_dump(\alpha\foo());
  var_dump(\alpha\STUFF);
  var_dump(alpha\vec());
  var_dump(alpha\foo());
  var_dump(alpha\STUFF);

  var_dump(\beta\vec\vec());
  var_dump(\beta\vec\foo());
  var_dump(\beta\vec\STUFF);
  var_dump(beta\vec\vec());
  var_dump(beta\vec\foo());
  var_dump(beta\vec\STUFF);

  vecnest(vec[vec[12]]);
}
}
