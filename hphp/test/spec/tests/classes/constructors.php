<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class D1
{
//  private function __construct($p1)
//  public function __construct($p1)
    protected function & __construct($p1)
    {
        echo "In D1 constructor, $p1\n";

//      return;
        $v = 123;
        return $v;      // Surprise; this is allowed and it works!!
    }
}

class D2 extends D1
{
///*
//  protected function __construct($p1, $p2)    // Access level to D2::__construct() must be public
    public function __construct($p1, $p2)
    {
        $v = parent::__construct($p1);
        var_dump($v);                       // I see the 123 returned!!
        echo "In D2 constructor, $p1, $p2\n";

//      return;
//      return 123;     // PHP5 does not diagnose this
    }
//*/
}

class D3 extends D2
{
///*
    public function __construct($p1, $p2, $p3)
    {
        parent::__construct($p1, $p2);
        echo "In D3 constructor, $p1, $p2, $p3\n";
//      parent::__construct($p1, $p2);      // not first statement in body, but OK
    }
//*/
}

class D4 extends D3
{
///*
    public function __construct()
    {
        parent::__construct(1,2,3);
        echo "In D4 constructor\n";
    }
//*/
}

//$d1 = new D1(10);
//$d2 = new D2(10, 20);
//$d3 = new D3(10, 20, 30);
$d4 = new D4;
