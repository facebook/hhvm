<?hh

function print_stuff($stuff)
:mixed{
    print $stuff;
}


function still_working()
:mixed{
    return "I'm still alive";
}

abstract final class DafnaStatics {
  public static $foo = 0;
}

function dafna()
:mixed{

    print "Dafna!\n";
    print call_user_func(still_working<>)."\n";
    DafnaStatics::$foo++;
    return (string) DafnaStatics::$foo;
}


class dafna_class {
    function __construct() {
        $this->myname = "Dafna";
    }
    function GetMyName() :mixed{
        return $this->myname;
    }
    function SetMyName($name) :mixed{
        $this->myname = $name;
    }
}

<<__EntryPoint>> function main(): void {
error_reporting(1023);

for ($i=0; $i<200; $i++) {
    print "$i\n";
    call_user_func(dafna<>);
    call_user_func(print_stuff<>,"Hey there!!\n");
    print "$i\n";
}

$dafna = new dafna_class();

print $name=call_user_func(vec[$dafna,"GetMyName"]);
print "\n";
}
