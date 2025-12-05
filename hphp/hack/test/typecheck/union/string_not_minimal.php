//// file1.php
<?hh

newtype ID_of<+Truntime as arraykey> as Truntime = Truntime;
newtype StrID as ID_of<string> = string;

//// file2.php
<?hh

function f((string | StrID) $x): void {
  // check that the union is simplified. Even though we make string a final class, it
  // shouldn't be consdered as such for the is_minimal helper in Typing_union, which is used in
  // union_list entrypoints used to simplify the explicit union syntax (and other places too)
  hh_show($x);
}

function g((string & KeyedContainer<arraykey, int>) $x): void {
  // We need to know that string and KeyedContainer are disjoint, although
  // other non-final classes wouldn't necessarily be.
  hh_show($x);
}
