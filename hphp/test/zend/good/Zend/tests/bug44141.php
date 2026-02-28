<?hh
class X
{
        public $x;
        private function __construct($x)
        {
                $this->x = $x;
        }
}

class Y extends X
{
        static public function cheat($x)
:mixed        {
                return new Y($x);
        }
}
<<__EntryPoint>> function main(): void {
$y = Y::cheat(5);
echo $y->x, PHP_EOL;
}
