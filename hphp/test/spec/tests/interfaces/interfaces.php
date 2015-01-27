<?php

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2014 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

error_reporting(-1);

interface iX
{
    const C1 = 123;
//  const C2 = "green";

    function f0 ();                 // implicitly public
    public function f1 ($p1);       // explicitly public
//  private function f1 ($p1);      // private not permitted
//  protected function f1 ($p1);    // protected not permitted
    public static function f3 ();
}

interface iY
{
//  const C1 = 123;         // can't inherit duplicate constants even if defined identically
    const C2 = "green";

//  function f0 ($p1);      // Declaration of iX::f0() must be compatible with iY::f0($p1)
    function f1 ($p1);      // implicitly public
    function f2 ($p1, $p2);
}

interface iZ extends iX, iY
{
//  const C1 = 123;         // can't override inherited constants
//  const C2 = "green";     // can't override inherited constants

    function f2 ($p1, $p2);
}

abstract class C implements iZ // being abstract, it need not implement any of the methods
{
}

class D implements iZ
{
//  function f0 ($p1) {}    // Declaration of D::f0() must be compatible with iX::f0()
    function f0 () {}
    public function f1 ($p1) {}
    function f2 ($p1, $p2) {}
    public static function f3 () {}
}

var_dump(D::C1);
var_dump(D::C2);

echo "------------------------------------\n";

include_once 'MyCollection.inc';
include_once 'MyList.inc';

class MyQueue implements MyCollection
{
    public function put($item)
    {
        // ...
    }

    public function get()
    {
        // ...
    }

    // ...
}

function processCollection(MyCollection $p1)
{
    var_dump($p1);
}

$list = new MyList;
processCollection($list);

$queue = new MyQueue;
processCollection($queue);

processCollection(new MyQueue);

var_dump(MyCollection::MAX_NUMBER_ITEMS);
var_dump(MyList::MAX_NUMBER_ITEMS);
var_dump(MyQueue::MAX_NUMBER_ITEMS);
