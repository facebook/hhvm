<?hh

function test<Ta, Tb, Tc<Td, Te>>() {}

class MyClass<Ta, Tb, Tc<Td as MyClass<Td>, Te> as OthererClassWithVeryLongNameLetsCauseALineBreak<Td>> {}

class MyClass2<Ta, Tb<Td as OthererClassWithVeryLongNameLetsCauseALineBreak<Td>, Tf as YetAnothererClassWithVeryLongNameLetsCauseALineBreak>, Tc<Tg as MyClass<Tg>, Te>> {}
