<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

class D1
{
//  private function __destruct()
    public function & __destruct()  // return by reference
//  protected function __destruct()
    {
        echo "In D1 destructor\n";

//      return;
        $v = 123;
        return $v;      // PHP5 does not diagnose this, and it works!!
    }
}

class D2 extends D1
{
///*
//  protected function __destruct() // Access level to D2::__destruct() must be public
    public function __destruct()
    {
        echo "In D2 destructor\n";
//      exit();
        $v = parent::__destruct();
        var_dump($v);                       // I see the 123 returned!!

//      return;
//      return 123;     // PHP5 does not diagnose this
    }
//*/
}

class D3 extends D2
{
///*
    public function __destruct()
    {
//      parent::__destruct();
        echo "In D3 destructor\n";
        parent::__destruct();
    }
//*/
}

class D4 extends D3
{
///*
    public function __destruct()
    {
        echo "In D4 destructor\n";
        parent::__destruct();
    }
//*/
}

//$d1 = new D1;
//$d2 = new D2;
//$d3 = new D3;
$d4 = new D4;
