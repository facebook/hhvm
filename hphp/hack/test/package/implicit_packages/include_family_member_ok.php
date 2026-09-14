//// prototypes/alpha/a.php
<?hh
class FamilyMemberClass {}

//// family_hard_consumer/use.php
<?hh
function use_family_member(): FamilyMemberClass {
  return new FamilyMemberClass();
}
