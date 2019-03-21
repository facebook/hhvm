<?hh


// Copyright (c) 2015 Thomas Punt
// https://github.com/tpunt/PHP7-Reference/blob/master/LICENSE.md
// https://github.com/tpunt/PHP7-Reference#preg_replace_callback_array-function

class TokenStream {
  // [tokenName, lexeme] pairs
  public $stream = vec[];
}

<<__EntryPoint>>
function main_preg_replace_callback_array_use_tokens() {
  $tokenStream = new TokenStream();

$input = <<<'end'
$a = 3; // variable initialisation
end;

$ret = preg_replace_callback_array(
  [
    '~\$[a-z_][a-z\d_]*~i' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = ['T_VARIABLE', $match[0]];
    },
    '~=~' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = ['T_ASSIGN', $match[0]];
    },
    '~[\d]+~' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = ['T_NUM', $match[0]];
    },
    '~;~' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = ['T_TERMINATE_STMT', $match[0]];
    },
    '~//.*~' => function ($match) use ($tokenStream) {
      $tokenStream->stream[] = ['T_COMMENT', $match[0]];
    }
  ],
  $input
);

var_dump($ret);
var_dump($tokenStream->stream);
}
