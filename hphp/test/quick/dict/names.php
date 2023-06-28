<?hh

namespace dict {
const STUFF = 12;
function dict() :mixed{ return 0; }
function foo() :mixed{ return 1; }
}

namespace alpha {
const STUFF = 12;
function dict() :mixed{ return 0; }
function foo() :mixed{ return 1; }
}

namespace beta\dict {
const STUFF = 12;
function dict() :mixed{ return 0; }
function foo() :mixed{ return 1; }
}

namespace {
function dictnest(dict<int, dict<int, int>> $foo) :mixed{}

function main() :mixed{
  var_dump(\dict\dict());
  var_dump(\dict\foo());
  var_dump(\dict\STUFF);

  var_dump(\alpha\dict());
  var_dump(\alpha\foo());
  var_dump(\alpha\STUFF);
  var_dump(alpha\dict());
  var_dump(alpha\foo());
  var_dump(alpha\STUFF);

  var_dump(\beta\dict\dict());
  var_dump(\beta\dict\foo());
  var_dump(\beta\dict\STUFF);
  var_dump(beta\dict\dict());
  var_dump(beta\dict\foo());
  var_dump(beta\dict\STUFF);

  dictnest(dict[1 => dict[2 => 3]]);
}
<<__EntryPoint>> function main_entry(): void {
main();
}
}
