<?hh

interface I {
  abstract const FOO;
}

interface I2 {
  abstract const BAR;
}

class A implements I {
  const FOO = 42;
}

class B implements I2, I {
  const BAR = 'lol';
  const FOO = 47;
}

function test(I $i) :mixed{
  var_dump($i::FOO);
}


<<__EntryPoint>>
function main_interface_diff_slots() :mixed{
test(new A());
test(new B());
}
