<?hh
class A extends \SplFileInfo
{
        public function __toString() :mixed{return ' -expected- ';}
}
<<__EntryPoint>> function main(): void {
$a = new A('/');

// Works
echo $a, $a->__toString(), $a->__toString() . '', "\n";

// Does not work - outputs parent::__toString()
echo $a . '', "\n";
}
