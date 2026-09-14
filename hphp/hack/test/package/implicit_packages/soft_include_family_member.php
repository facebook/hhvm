//// prototypes/alpha/a.php
<?hh
class SoftFamilyMemberClass {}

//// family_soft_consumer/use.php
<?hh
function use_soft_family_member(): void {
  new SoftFamilyMemberClass();
}
