<?hh

class StayClassy<Ta,Tb> {
  private ?Ta;
  private ?Tb;
}

function tycon_arity_user_defined0(StayClassy $x): void {}

function tycon_arity_user_defined1(StayClassy<int> $x): void {}

function tycon_arity_user_defined2(StayClassy<int,bool> $x): void {}

function tycon_arity_user_defined3(StayClassy<int,bool,string> $x): void {}
