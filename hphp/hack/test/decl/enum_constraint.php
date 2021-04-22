<?hh

enum A : string as string {X='a';}

enum B : int as IdOf<X> {X=1;}

enum C : int as arraykey {X=1;}

newtype N as arraykey = int;

enum D : int as N {X=1; Y="a";}

enum E : C as C {X=C::X;}
