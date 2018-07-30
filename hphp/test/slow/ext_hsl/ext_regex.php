<?hh

use namespace HH\Lib\Regex;

function f(Regex\Pattern $pattern): void {
  echo($pattern);
}

f("hello\n");
