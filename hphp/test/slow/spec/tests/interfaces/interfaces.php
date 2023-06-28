<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/

interface iX
{
    const C1 = 123;
//  const C2 = "green";

    function f0 ():mixed;                 // implicitly public
    public function f1 ($p1):mixed;       // explicitly public
//  private function f1 ($p1);      // private not permitted
//  protected function f1 ($p1);    // protected not permitted
    public static function f3 ():mixed;
}

interface iY
{
//  const C1 = 123;         // can't inherit duplicate constants even if defined identically
    const C2 = "green";

//  function f0 ($p1);      // Declaration of iX::f0() must be compatible with iY::f0($p1)
    function f1 ($p1):mixed;      // implicitly public
    function f2 ($p1, $p2):mixed;
}

interface iZ extends iX, iY
{
//  const C1 = 123;         // can't override inherited constants
//  const C2 = "green";     // can't override inherited constants

    function f2 ($p1, $p2):mixed;
}

abstract class C implements iZ // being abstract, it need not implement any of the methods
{
}

class D implements iZ
{
//  function f0 ($p1) {}    // Declaration of D::f0() must be compatible with iX::f0()
    function f0 () :mixed{}
    public function f1 ($p1) :mixed{}
    function f2 ($p1, $p2) :mixed{}
    public static function f3 () :mixed{}
}

function processCollection(MyCollection $p1)
:mixed{
    var_dump($p1);
}
<<__EntryPoint>> function main(): void {
  include_once 'MyCollection.inc';
  include_once 'MyList.inc';
  include_once 'interfaces.inc';

error_reporting(-1);

var_dump(D::C1);
var_dump(D::C2);

echo "------------------------------------\n";

$list = new MyList;
processCollection($list);

$queue = new MyQueue;
processCollection($queue);

processCollection(new MyQueue);

var_dump(MyCollection::MAX_NUMBER_ITEMS);
var_dump(MyList::MAX_NUMBER_ITEMS);
var_dump(MyQueue::MAX_NUMBER_ITEMS);
}
