<?hh


// Copyright (c) 2015 Thomas Punt
// https://github.com/tpunt/PHP7-Reference/blob/master/LICENSE.md
// https://github.com/tpunt/PHP7-Reference#preg_replace_callback_array-function

class TokenStream {
  // [tokenName, lexeme] pairs
  public $stream = vec[];
}

<<__EntryPoint>>
function main_preg_replace_callback_array_use_tokens() :mixed{
  $tokenStream = new TokenStream();

$input = <<<'end'
$a = 3; // variable initialisation
end;

$count = -1;
$ret = preg_replace_callback_array(
  dict[
    '~\$[a-z_][a-z\d_]*~i' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = vec['T_VARIABLE', $match[0]];
    },
    '~=~' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = vec['T_ASSIGN', $match[0]];
    },
    '~[\d]+~' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = vec['T_NUM', $match[0]];
    },
    '~;~' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = vec['T_TERMINATE_STMT', $match[0]];
    },
    '~//.*~' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = vec['T_COMMENT', $match[0]];
    }
  ],
  $input,
  -1,
  inout $count,
);

var_dump($ret);
var_dump($tokenStream->stream);
}
