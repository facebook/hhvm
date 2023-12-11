<?hh

class C
{
  function m1()
:mixed  {
    $this->m2();
  }

  function m2()
:mixed  {
    $this->m3();
  }

  function m3()
:mixed  {
    $this->m4();
  }

  function m4()
:mixed  {
    $this->m5();
  }

  function m5()
:mixed  {
    $this->m6();
  }

  function m6()
:mixed  {
    $this->m7();
  }

  function m7()
:mixed  {
    $this->m8();
  }

  function m8()
:mixed  {
    $this->m9();
  }

  function m9()
:mixed  {
    $this->m10();
  }

  function m10()
:mixed  {
    $this->m11(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  }

  function m11($a1, $a2, $a3, $a4, $a5, $a6, $a7, $a8, $a9, $a10)
:mixed  {
    $arr = explode('a', 'b');
  }
}

function f($str)
:mixed{
  $obj = new C;
  $obj->m1();
  return TRUE;
}

function p5($a1, $a2, $a3, $a4, $a5, $a6, $a7, $a8, $a9, $a10, $a11, $a12)
:mixed{
  $ret = array_filter(vec[0], f<>);
}

function p4()
:mixed{
  p5(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
}

function p3()
:mixed{
  p4();
}

function p2()
:mixed{
  p3();
}

function p1()
:mixed{
  p2();
}
<<__EntryPoint>> function main(): void {
p1();
echo "ok\n";
}
