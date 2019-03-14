<?hh // partial

# taken from an issue in GitHub
#
function add_word_helper<T>((function(): T) $callback): T {
  return $callback();
}

function add_word(array<string> $words): array<string> {
  return add_word_helper(() ==> {
    if (count($words) == 0){
      $words = ['current'];
    }

    $words[] = 'events';

    return $words;
  });
}

var_dump(add_word(['when', 'in', 'the', 'course', 'of', 'human']));
