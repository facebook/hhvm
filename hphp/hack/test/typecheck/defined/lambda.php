<?hh // partial

# taken from an issue in GitHub
#
function add_word_helper<T>((function(): T) $callback): T {
  return $callback();
}

function add_word(varray<string> $words): varray<string> {
  return add_word_helper(() ==> {
    if (count($words) == 0){
      $words = varray['current'];
    }

    $words[] = 'events';

    return $words;
  });
}

var_dump(add_word(varray['when', 'in', 'the', 'course', 'of', 'human']));
