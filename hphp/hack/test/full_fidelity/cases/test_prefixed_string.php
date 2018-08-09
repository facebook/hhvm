<?hh

function f() : void {
  $x = "It";
  $y = "worst";
  $z = f"{$x} was the {$y} of times";
  $w = re"blah blah";
  $a = re"";
  // Consistent with `aks-A124jk12jhd()`; Parses as `aks` minus prefixed string
  $a0 = aks-A124jk12jhd"sosdk!nwekje@";
  // Parses as prefixed string
  $a1 = aksA124jkhgf12jhdddsljs"sosdk!nwekje@";
  // Parser error; `re!interpolator` is not a name
  $a2 = re!interpolator"Hello";
  $b = re"//";
  $b0 = aaaaaa"aaaaaa";
  $c = re"/(.?)/";
  $d = re"/.*/";
  $e = $c.$d;
  $f = re"//".re"//";
  $g = Regex\re"//";
  $h = Foo\FooInterpolator"";
  // TODO(T19708752): Enable qualified names as prefixes
  // $i = \HH\Lib\Private\Foo\FooInterpolator"";
  // $j = Private\Foo\FooInterpolator"";
  // $h0 = Foo\FooInterpolator"hello$firstname{$lastname}, goodbye";
  // $i0 = \HH\Lib\Private\Foo\FooInterpolator"hello$x, goodbye";
  // $j0 = Private\Foo\FooInterpolator"hello$x, goodbye";
}
