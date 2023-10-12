<?hh

function f<<<__Attr>> reify T>() {}
function g< <<__Attr>> reify U>() {}
function h<reify V>() {}

class C {
  const type TyC<<<Attr>> T> = int;
  public function ff<<<__Attr>> reify Tt>() {}
  public function gg< <<__Attr>> reify Uu>() {}
  public function hh<reify Vv>() {}
}

class CC<<<__Attr>> reify Tt>{}
class DD< <<__Attr>> reify Tt>{}
class EE<reify Tt>{}

type Ty<<<Attr>> T> = int;
newtype Ty2<<<Attr>> T> = int;
