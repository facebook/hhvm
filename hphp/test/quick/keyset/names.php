<?hh

namespace keyset {
const STUFF = 12;
function keyset() :mixed{ return 0; }
function foo() :mixed{ return 1; }
}

namespace alpha {
const STUFF = 12;
function keyset() :mixed{ return 0; }
function foo() :mixed{ return 1; }
}

namespace beta\keyset {
const STUFF = 12;
function keyset() :mixed{ return 0; }
function foo() :mixed{ return 1; }
}

namespace {

<<__EntryPoint>>
function main() :mixed{
  var_dump(\keyset\keyset());
  var_dump(\keyset\foo());
  var_dump(\keyset\STUFF);

  var_dump(\alpha\keyset());
  var_dump(\alpha\foo());
  var_dump(\alpha\STUFF);
  var_dump(alpha\keyset());
  var_dump(alpha\foo());
  var_dump(alpha\STUFF);

  var_dump(\beta\keyset\keyset());
  var_dump(\beta\keyset\foo());
  var_dump(\beta\keyset\STUFF);
  var_dump(beta\keyset\keyset());
  var_dump(beta\keyset\foo());
  var_dump(beta\keyset\STUFF);
}

}
