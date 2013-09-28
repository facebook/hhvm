<?php

abstract class TB {
  const PARAM_A = 'aaa';
  const PARAM_B = 'bbb';
  const PARAM_C = 'ccc';
  const PARAM_D = 'ddd';
}
abstract class ATB extends TB {
}
class ABCD extends ATB {
  static public function foo() {
    return array(      'a_ids'   => array(        ATB::PARAM_A => true,        ATB::PARAM_C   => array(          array('tcks', 'none'),          array('tcks', 'ids'),          ),        ATB::PARAM_B     =>          'aaaa',      ),      'user_id'   => array(        ATB::PARAM_A => true,        ATB::PARAM_C   => array(          array('tcks', 'none'),          array('tcks', 'id'),          ),        ATB::PARAM_B     =>          'bbbb',      ),    );
  }
}
var_dump(ABCD::foo());
