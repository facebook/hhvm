<?hh
namespace foo\foo;

const C = "foo\\foo\\C\n";
const I = 12;

class foo {
  const I = 32;
  const C = "foo\\foo\\foo::C\n";
}

namespace foo;
use \RegexIterator;

const C = "foo\\C\n";
const I = 11;

class foo {
    const C = "foo\\foo::C\n";
    const I = 22;
    const C1 = C;
    const C2 = foo\C;
    const C3 = foo\foo::C;
    const C4 = \foo\C;
    const C5 = \foo\foo::C;
    const C6 = RegexIterator::GET_MATCH;
    const C7 = \E_ERROR;
}

class bar1 {
    public static $a1 = darray[I => 0];
    public static $a2 = darray[foo\I => 0];
    public static $a3 = darray[foo\foo::I => 0];
    public static $a4 = darray[\foo\I => 0];
    public static $a5 = darray[\foo\foo::I => 0];
    public static $a6 = darray[RegexIterator::GET_MATCH => 0];
    public static $a7 = darray[\E_ERROR => 0];
}

class bar2 {
    public static $a1 = darray[I => I];
    public static $a2 = darray[foo\I => I];
    public static $a3 = darray[foo\foo::I => I];
    public static $a4 = darray[\foo\I => I];
    public static $a5 = darray[\foo\foo::I => I];
    public static $a6 = darray[RegexIterator::GET_MATCH => I];
    public static $a7 = darray[\E_ERROR => I];
}

class bar3 {
    public static $a1 = darray[I => foo\I];
    public static $a2 = darray[foo\I => foo\I];
    public static $a3 = darray[foo\foo::I => foo\I];
    public static $a4 = darray[\foo\I => foo\I];
    public static $a5 = darray[\foo\foo::I => foo\I];
    public static $a6 = darray[RegexIterator::GET_MATCH => foo\I];
    public static $a7 = darray[\E_ERROR => foo\I];
}

class bar4 {
    public static $a1 = darray[I => RegexIterator::GET_MATCH];
    public static $a2 = darray[foo\I => RegexIterator::GET_MATCH];
    public static $a3 = darray[foo\foo::I => RegexIterator::GET_MATCH];
    public static $a4 = darray[\foo\I => RegexIterator::GET_MATCH];
    public static $a5 = darray[\foo\foo::I => RegexIterator::GET_MATCH];
    public static $a6 = darray[RegexIterator::GET_MATCH => RegexIterator::GET_MATCH];
    public static $a7 = darray[\E_ERROR => RegexIterator::GET_MATCH];
}

class bar5 {
    public static $a1 = darray[I => \E_ERROR];
    public static $a2 = darray[foo\I => \E_ERROR];
    public static $a3 = darray[foo\foo::I => \E_ERROR];
    public static $a4 = darray[\foo\I => \E_ERROR];
    public static $a5 = darray[\foo\foo::I => \E_ERROR];
    public static $a6 = darray[RegexIterator::GET_MATCH => \E_ERROR];
    public static $a7 = darray[\E_ERROR => \E_ERROR];
}

function oops($a = varray[foo\unknown]):mixed{}

<<__EntryPoint>> function main(): void {
echo "first\n";
echo C;
echo foo\C;
echo foo\foo::C;
echo foo::C;
echo \foo\foo::C;
echo RegexIterator::GET_MATCH . "\n";
echo \E_ERROR . "\n";
echo "second\n";
echo \foo\foo::C1;
echo \foo\foo::C2;
echo \foo\foo::C3;
echo \foo\foo::C4;
echo \foo\foo::C5;
echo \foo\foo::C6 . "\n";
echo \foo\foo::C7 . "\n";

\print_r(bar1::$a1);
\print_r(bar1::$a2);
\print_r(bar1::$a3);
\print_r(bar1::$a4);
\print_r(bar1::$a5);
\print_r(bar1::$a6);
\print_r(bar1::$a7);

\print_r(bar2::$a1);
\print_r(bar2::$a2);
\print_r(bar2::$a3);
\print_r(bar2::$a4);
\print_r(bar2::$a5);
\print_r(bar2::$a6);
\print_r(bar2::$a7);

\print_r(bar3::$a1);
\print_r(bar3::$a2);
\print_r(bar3::$a3);
\print_r(bar3::$a4);
\print_r(bar3::$a5);
\print_r(bar3::$a6);
\print_r(bar3::$a7);

\print_r(bar4::$a1);
\print_r(bar4::$a2);
\print_r(bar4::$a3);
\print_r(bar4::$a4);
\print_r(bar4::$a5);
\print_r(bar4::$a6);
\print_r(bar4::$a7);

\print_r(bar5::$a1);
\print_r(bar5::$a2);
\print_r(bar5::$a3);
\print_r(bar5::$a4);
\print_r(bar5::$a5);
\print_r(bar5::$a6);
\print_r(bar5::$a7);

oops();
}
