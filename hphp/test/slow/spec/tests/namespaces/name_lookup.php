<?hh

///*
function f1()
:mixed{
    echo "Inside function " . __FUNCTION__ . "\n";
}
//*/

/*
namespace NS1;

function f2()
{
    echo "Inside function " . __FUNCTION__ . "\n";
}

f2();           // implicitly in current namespace
namespace\f2(); // explicitly in current namespace
\NS1\f2();      // explicitly in given namespace

//NS1\f2(); // looking for relative name NS1\NS1\f2(), which doesn't exist
//\namespace\f2();  // namespace keyword can only be a prefix
*/
<<__EntryPoint>>
function main_entry(): void {
  error_reporting(-1);

  f1();           // implicitly in current namespace
  namespace\f1(); // explicitly in current namespace
  \f1();          // explicitly in top-level scope
}
