<?hh // strict

class :foo extends XHPTest {
  attribute
    int myint,
    string mystring,
    float myfloat,
    bool mybool;
}

function main(): void {
  $x = <foo myint={123} mystring="herp" myfloat={1.23} mybool={true} />;
  // Also fine because the attributes aren't required and don't have defaults
  $y = <foo myint={null} mystring={null} myfloat={null} mybool={null} />;
}
