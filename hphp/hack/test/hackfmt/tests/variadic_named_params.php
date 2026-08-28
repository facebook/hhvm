<?hh

function unnamed(int $x, named    arraykey   ...): void {}

function both(int $x, string   ...$rest,    named arraykey...): void {}

abstract class C {
  abstract public function m(named
    bool...): void;

  public static function long_function_name_with_lots_of_args(int $a, string $b, dict<int, vec<string>> $c, bool ...$d, named arraykey...): void {}
}

function in_lambdas(): void {
  $_ = (int $x, named   arraykey...) ==> $x;
  $_ = function(named int ...): void {};
  $_ = async (named int...) ==> 1;
  $_=(int $first,string $second,bool $third,named float $flag,named dict<string,vec<int>>
...
)==>$first;
}

type TNamedVariadicFn=(function(named   int ...):void);

function accepts((function(int,string,bool,float,named arraykey $key,named dict<string,vec<int>>...):void)$f):void{}

function returns():(function(int,string...,named bool...):void){}
