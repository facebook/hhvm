<?hh
interface constr
{
    function __construct();
}

abstract class implem implements constr
{
}

class derived extends implem
{
    function __construct($a)
    {
    }
}

<<__EntryPoint>>
function main(): void {
  echo "Done.\n";
}
