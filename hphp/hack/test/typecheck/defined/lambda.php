<?hh

// taken from an issue in GitHub
//
function add_word_helper<T>((function(): T) $callback): T {
  return $callback();
}

function add_word(varray<string> $words): varray<string> {
  return add_word_helper(() ==> {
    if (count($words) == 0){
      $words = vec['current'];
    }

    $words[] = 'events';

    return $words;
  });
}

function main(): void {
  var_dump(add_word(vec['when', 'in', 'the', 'course', 'of', 'human']));
}
