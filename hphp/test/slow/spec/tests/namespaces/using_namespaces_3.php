<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

namespace NS1
{
    const CON1 = 100;

    function f()
    {
        echo "In " . __FUNCTION__ . "\n";
    }

    class C
    {
        const C_CON = 200;
        public function f()
        {
            echo "In " . __NAMESPACE__ . "..." . __METHOD__ . "\n";
        }
    }

    interface I
    {
        const I_CON = 300;
    }

    trait T
    {
        public function f()
        {
            echo "In " . __TRAIT__ . "..." . __NAMESPACE__ . "..." . __METHOD__ . "\n";
        }
    }
}

namespace NS2
{
/*
    echo "CON1 = " . \NS1\CON1 . "\n";

    \NS1\f();

    class D extends \NS1\C implements \NS1\I
    {
        use \NS1\T;
    }

    $d = new D;
    var_dump($d);

echo "=============================================\n";
*/
    use \NS1\C, \NS1\I, \NS1\T;

    class D extends C implements I
    {
        use T;
    }

    $d = new D;
    var_dump($d);

    echo "CON1 = " . \NS1\CON1 . "\n";

    \NS1\f();

    use \NS1\C as C2;
    $c2 = new C2;
    var_dump($c2);
}
