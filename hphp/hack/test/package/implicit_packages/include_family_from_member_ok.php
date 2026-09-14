//// prototypes/alpha/a.php
<?hh
class FamilyTargetClass {}

//// family_consumers/beta/use.php
<?hh
function use_family_member_from_member(): FamilyTargetClass {
  return new FamilyTargetClass();
}
