//// file1.php
<?hh

newtype ID_of<+Truntime as arraykey> as Truntime = Truntime;
newtype StrID as ID_of<string> = string;

//// file2.php
<?hh

function f((string | StrID) $x): void {
  // check that the union is simplified. If we make string a final class, it
  // won't be, at least for union_list entrypoints
  hh_show($x);
}

function g((string & KeyedContainer<arraykey, int>) $x): void {
  // We need to know that string and KeyedContainer are disjoint, although
  // other non-final classes wouldn't necessarily be.
  hh_show($x);
}
