<?hh

class :xhp extends XHPTest {
  attribute string attr;
}

function main1()[]:void {
  <xhp />;
}

function main2(:xhp $xhp)[]:void {
  $xhp->:attr;
}
