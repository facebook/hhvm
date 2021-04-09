<?hh

class TestDict{
  public function n(dict<int, int> $n): void {}
}

function n(dict<int, int> $n): void {}

function call_dict(
  dict<string, int> $n1,
  dict<int, string> $n2,
): void {
  /* HH_FIXME[4110] */
  n($n1);
  /* HH_FIXME[4110] */
  n($n2);

  $foo = new TestDict();

  /* HH_FIXME[4110] */
  $foo->n($n1);
  /* HH_FIXME[4110] */
  $foo->n($n2);
}

function call_dict_cast(
  dict<string, int> $n1,
  dict<int, string> $n2,
): void {
  /* HH_FIXME[4417] */
  n(unsafe_cast<dict<string,int>,dict<int,int>>($n1));
  /* HH_FIXME[4417] */
  n(unsafe_cast<dict<int,string>,dict<int,int>>($n2));

  $foo = new TestDict();

  /* HH_FIXME[4417] */
  $foo->n(unsafe_cast<dict<string,int>,dict<int,int>>($n1));
  /* HH_FIXME[4417] */
  $foo->n(unsafe_cast<dict<int,string>,dict<int,int>>($n2));
}
