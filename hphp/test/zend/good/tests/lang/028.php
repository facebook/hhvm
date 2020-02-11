<?hh

function print_stuff($stuff)
{
    print $stuff;
}


function still_working()
{
    return "I'm still alive";
}

abstract final class DafnaStatics {
  public static $foo = 0;
}

function dafna()
{

    print "Dafna!\n";
    print call_user_func(fun("still_working"))."\n";
    DafnaStatics::$foo++;
    return (string) DafnaStatics::$foo;
}


class dafna_class {
    function __construct() {
        $this->myname = "Dafna";
    }
    function GetMyName() {
        return $this->myname;
    }
    function SetMyName($name) {
        $this->myname = $name;
    }
};

<<__EntryPoint>> function main(): void {
error_reporting(1023);

for ($i=0; $i<200; $i++) {
    print "$i\n";
    call_user_func(fun("dafna"));
    call_user_func(fun("print_stuff"),"Hey there!!\n");
    print "$i\n";
}

$dafna = new dafna_class();

print $name=call_user_func(varray[$dafna,"GetMyName"]);
print "\n";
}
