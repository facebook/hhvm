<?php

function z() {
  var_dump('__autoload');
  var_dump(class_exists('cNew'));
  var_dump(class_exists('cNew_r'));
  var_dump(class_exists('cNew_d'));
  var_dump(class_exists('csm'));
  var_dump(class_exists('csm_r'));
  var_dump(class_exists('CcOn'));
  var_dump(class_exists('CcOn_r'));
  var_dump(class_exists('CcOn_d'));
  var_dump(class_exists('csmeth'));
  var_dump(class_exists('csmeth_r'));
  var_dump(class_exists('csmeth_d'));
  var_dump(class_exists('cpar'));
  var_dump(class_exists('cpar_r'));
  var_dump(class_exists('cref'));
  var_dump(class_exists('cex'));
}
function test() {
  function t1() {
    new cNew();
  }
  t1();
  function t2() {
    new cNew_r();
  }
  t2();
  function t4() {
    $x = 'cNew_d';
    new $x();
  }
  t4();
  function t5() {
    var_dump(csm::$mem);
  }
  t5();
  function t6() {
    var_dump(csm_r::$mem);
  }
  t6();
  function t7() {
    var_dump(CcOn::C);
  }
  t7();
  function t8() {
    var_dump(CcOn_r::C);
  }
  t8();
  function t9() {
    var_dump(constant('CcOn_d::C'));
  }
  t9();
  function t10() {
    csmeth::m();
  }
  t10();
  function t11() {
    csmeth_r::m();
  }
  t11();
  function t12() {
    call_user_func(array('csmeth_d', 'm'));
  }
  t12();
  function t13() {
    class a extends cpar {
}
    new a;
  }
  t13();
  function t14() {
    class b extends cpar_r {
}
    new b;
  }
  t14();
  function t15() {
    new ReflectionClass('cref');
  }
  t15();
  function t16() {
    var_dump(class_exists('cex'));
  }
  t16();
  function t17() {
    var_dump(class_exists('cex_r'));
  }
  t17();
}
test();
z();
function __autoload($name) {
  var_dump('autoload ' . $name);
  switch ($name) {
    case 'cNew':      class cNew {
}
      break;
    case 'cNew_r':      class cNew_r {
}
      if (false) {
        class cNew_r {
}
      }
      break;
    case 'cNew_d':      class cNew_d {
}
      break;
    case 'csm':      class csm {
        public static $mem = 1;
      }
      break;
    case 'csm_r':      class csm_r {
        public static $mem = 1;
      }
      if (false) {
        class csm_r {
}
      }
      break;
    case 'CcOn':      class CcOn {
        const C = 2;
      }
      break;
    case 'CcOn_r':      class CcOn_r {
        const C = 2;
      }
      if (false) {
        class CcOn_r {
}
      }
      break;
    case 'CcOn_d':      class CcOn_d {
        const C = 2;
      }
      break;
    case 'csmeth':      class csmeth {
        public static function m() {
 echo '1
';
 }
      }
      break;
    case 'csmeth_r':      class csmeth_r {
        public static function m() {
 echo '1';
 }
      }
      if (false) {
        class csmeth_r {
}
      }
      break;
    case 'csmeth_d':      class csmeth_d {
        public static function m() {
 echo '1';
 }
      }
      break;
    case 'cpar':      class cpar {
}
      break;
    case 'cpar_r':      class cpar_r {
}
      if (false) {
        class cpar_r {
}
      }
      break;
    case 'cref':      class cref {
}
      break;
    case 'cex':      class cex {
}
      break;
    case 'cex_r':      class cex_r {
}
      if (false) {
        class cex_r {
}
      }
      break;
  }
}
