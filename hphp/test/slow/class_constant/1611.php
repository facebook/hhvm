<?hh

abstract class TB {
  const PARAM_A = 'aaa';
  const PARAM_B = 'bbb';
  const PARAM_C = 'ccc';
  const PARAM_D = 'ddd';
}
abstract class ATB extends TB {
}
class ABCD extends ATB {
  static public function foo() :mixed{
    return dict[      'a_ids'   => dict[        ATB::PARAM_A => true,        ATB::PARAM_C   => vec[          vec['tcks', 'none'],          vec['tcks', 'ids'],          ],        ATB::PARAM_B     =>          'aaaa',      ],      'user_id'   => dict[        ATB::PARAM_A => true,        ATB::PARAM_C   => vec[          vec['tcks', 'none'],          vec['tcks', 'id'],          ],        ATB::PARAM_B     =>          'bbbb',      ],    ];
  }
}

<<__EntryPoint>>
function main_1611() :mixed{
var_dump(ABCD::foo());
}
