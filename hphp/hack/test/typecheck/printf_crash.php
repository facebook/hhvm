//// def.php
<?hh // strict

newtype FormatString<T> = string;

//// printf.php
<?hh // partial
interface MyPlainSprintf {
}

function my_sprintf(FormatString<PlainSprintf> $f, ...$_): string;

//// use.php
<?hh // strict

final class Foo {
  public function f(FormatString $s): void {
    my_sprintf($s);
  }
}
