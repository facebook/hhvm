//// prototypes/alpha/a.php
<?hh
class IncludedMemberClass {}

//// prototypes/beta/b.php
<?hh
class ExcludedMemberClass {}

//// member_consumer/use.php
<?hh
function use_included_member(): void {
  new IncludedMemberClass();
}

function use_excluded_member(): void {
  new ExcludedMemberClass();
}
