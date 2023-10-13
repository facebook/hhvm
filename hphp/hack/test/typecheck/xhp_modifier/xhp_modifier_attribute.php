<?hh

xhp class foo extends XHPTest {
    // Verify that attributes are allowed.
    attribute int userid @required;
    // Verify that attributes are usable
    public function getUserID(): int {
      return $this->:userid;
    }
}
