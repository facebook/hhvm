//// def.php
<?hh // strict

newtype FormatString<T> = string;

//// printf.php
<?hh // decl
interface PlainSprintf {
}

function sprintf(FormatString<PlainSprintf> $f, ...): string;

//// use.php
<?hh // strict

final class Foo {
  public function f(FormatString $s): void {
    sprintf($s);
  }
}
