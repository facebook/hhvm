//// base-a.php
<?hh
enum AvBase: int as int {
  A = 1;
  B = 2;
}
//// base-b.php
<?hh
// AvAlias is exempt while AvBase's aliased members have distinct values.
// Changing AvBase::B to duplicate AvBase::A makes AvBase no longer clean, so
// AvAlias loses its exemption -- hence the fanout re-checks both AvBase and
// AvAlias.
enum AvAlias: int as int {
  X = AvBase::A;
  Y = AvBase::B;
}

//// changed-a.php
<?hh
enum AvBase: int as int {
  A = 1;
  B = 1;
}
//// changed-b.php
<?hh
enum AvAlias: int as int {
  X = AvBase::A;
  Y = AvBase::B;
}
