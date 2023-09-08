//// modules.php
<?hh

new module here {}
new module there {}
new module elsewhere {}

//// here.php
<?hh
module here;

function foo():void { }

type Talias = int;

newtype Topaque = string;

//// there.php
<?hh
module there;

class C {
  public function bar():void { }
}

//// elsewhere.php
<?hh
module elsewhere;


enum E : int {
  A = 3;
}
