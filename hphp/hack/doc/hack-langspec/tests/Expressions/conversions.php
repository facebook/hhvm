<?hh // strict

namespace NS_conversions;

class Button {
  private string $label;
  public function __construct(string $label = '??') {
    $this->label = $label;
  }
  public function __toString(): string {
    return '[[Button: ' . $this->label . ']]';
  }
}

class CustomButton extends Button {
  public function __construct(string $label = '??') {
    parent::__construct($label);
  }
  public function __toString(): string {
    return parent::__toString() . '[[CustomButton]]';
  }
}

interface MyCollection {}

class MyList implements MyCollection {
  public function __toString(): string {
    return '[[MyList]]';
  }
}

class MyQueue implements MyCollection {
  public function __toString(): string {
    return '[[MyQueue]]';
  }
}

function get_resource(): resource {
  return STDOUT;
}

class C1 {
  private bool $pr_bool;
  private int $pr_int;
  private float $pr_float;
  private num $pr_num;
  private string $pr_string;
  private arraykey $pr_arraykey;
  private resource $pr_resource;
  private Button $pr_Button;
  private CustomButton $pr_CustomButton;
  private MyCollection $pr_MyCollection;

  private ?bool $pr_nbool = null;
  private ?int $pr_nint = null;
  private ?float $pr_nfloat = null;
  private ?num $pr_nnum = null;
  private ?string $pr_nstring = null;
  private ?resource $pr_nresource = null;
  private ?Button $pr_nButton = null;
  private ?CustomButton $pr_nCustomButton = null;
  private ?MyCollection $pr_nMyCollection = null;

  private mixed $pr_mixed = null;

  public function __construct() {
//  echo "================ Testing (unset) Cast ===================\n\n";
//
//  $this->pr_bool = (unset)false;
//  echo "(unset)false: pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";
//  $this->pr_float = (unset)456.78;
//  echo "(unset)456.78: pr_float = >" . $this->pr_float . "<\n";

    echo "\n================ setting pr_bool ===================\n\n";

    $this->pr_bool = false;		// false
    $this->pr_bool = (bool)false;	// false
    echo "(bool)false: pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

    $this->pr_bool = true;		// true
    $this->pr_bool = (bool)true;	// true
    echo "(bool)true: pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = 0;		// int is incompatible with bool
    $this->pr_bool = (bool)0;	// false
    echo "(bool)0: pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = 100;		// int is incompatible with bool
    $this->pr_bool = (bool)100;	// true
    echo "(bool)100: pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = 0.0;		// float is incompatible with bool
    $this->pr_bool = (bool)0.0;	// false
    echo "(bool)0.0: pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = 123.45;	// float is incompatible with bool
    $this->pr_bool = (bool)123.45;	// true
    echo "(bool)123.45: pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = "";		// string is incompatible with bool
    $this->pr_bool = (bool)"";	// false
    echo "(bool)\"\": pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = "0";		// string is incompatible with bool
    $this->pr_bool = (bool)"0";	// false
    echo "(bool)\"0\": pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = "123ABC";	// string is incompatible with bool
    $this->pr_bool = (bool)"123ABC";	// true
    echo "(bool)\"123ABC\": pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = "456.78ABC";	// string is incompatible with bool
    $this->pr_bool = (bool)"456.78ABC";	// true
    echo "(bool)\"456.78ABC\": pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = null;		// null is incompatible with bool
    $this->pr_bool = (bool)null;	// false
    echo "(bool)null: pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = array();	// array() is incompatible with bool
    $this->pr_bool = (bool)array();	// false
    echo "(bool)array(): pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//   $this->pr_bool = array(10,20,30);	// array(...) is incompatible with bool
    $this->pr_bool = (bool)array(10,20,30);	// true
    echo "(bool)array(10,20,30): pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//    $this->pr_bool = new Button();		// Button is incompatible with bool
    $this->pr_bool = (bool)new Button();	// true
    echo "(bool)new Button(): pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

//   $this->pr_bool = get_resource();	// resource is not compatible with bool
    $this->pr_bool = (bool)get_resource();	// true
    echo "(bool)resource: pr_bool = >" . ($this->pr_bool ? "true" : "false") . "<\n";

    $this->pr_arraykey = 0;
//    $this->pr_bool = $this->pr_arraykey;	// arraykey is incompatible with bool
    $this->pr_bool = (bool)$this->pr_arraykey;	// 
    echo "(bool)arraykey: pr_arraykey = >" . ($this->pr_bool ? "true" : "false") . "<\n";

    $this->pr_arraykey = 10;
    $this->pr_bool = (bool)$this->pr_arraykey;	// 
    echo "(bool)arraykey: pr_arraykey = >" . ($this->pr_bool ? "true" : "false") . "<\n";

    $this->pr_arraykey = "";
//    $this->pr_bool = $this->pr_arraykey;	// arraykey is incompatible with bool
    $this->pr_bool = (bool)$this->pr_arraykey;	// 
    echo "(bool)arraykey: pr_arraykey = >" . ($this->pr_bool ? "true" : "false") . "<\n";

    $this->pr_arraykey = "ABC";
    $this->pr_bool = (bool)$this->pr_arraykey;	// 
    echo "(bool)arraykey: pr_arraykey = >" . ($this->pr_bool ? "true" : "false") . "<\n";


    echo "\n================ setting pr_int ===================\n\n";

//    $this->pr_int = false;		// bool is incompatible with int
    $this->pr_int = (int)false;	// 0
    echo "(int)false: pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = true;		// bool is incompatible with int
    $this->pr_int = (int)true;	// 1
    echo "(int)true: pr_int = >" . $this->pr_int . "<\n";

    $this->pr_int = 0;		// 0
    $this->pr_int = (int)0;		// 0
    echo "(int)0: pr_int = >" . $this->pr_int . "<\n";

    $this->pr_int = 100;		// 100
    $this->pr_int = (int)100;	// 100
    echo "(int)100: pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = 0.0;		// float is incompatible with int
    $this->pr_int = (int)0.0;	// 0
    echo "(int)0.0: pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = 123.45;		// float is incompatible with int
    $this->pr_int = (int)123.45;	// 123
    echo "(int)123.45: pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = "";		// string is incompatible with int
    $this->pr_int = (int)"";	// 0
    echo "(int)\"\": pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = "0";		// string is incompatible with int
    $this->pr_int = (int)"0";	// 0
    echo "(int)\"0\": pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = "123ABC";	// string is incompatible with int
    $this->pr_int = (int)"123ABC";	// 123
    echo "(int)\"123ABC\": pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = "456.78ABC";	// string is incompatible with bool
    $this->pr_int = (int)"456.78ABC";	// 456
    echo "(int)\"456.78ABC\": pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = null;		// null is incompatible with int
    $this->pr_int = (int)null;	// 0
    echo "(int)null: pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = array();	// array() is incompatible with int
    $this->pr_int = (int)array();	// 0
 echo "(int)array(): pr_int = >" . $this->pr_int . "<\n";

// $this->pr_int = array(10,20,30);	// array(...) is incompatible with int
    $this->pr_int = (int)array(10,20,30);	// 1
    echo "(int)array(10,20,30): pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = new Button();		// Button is incompatible with int
//    $this->pr_int = (int)new Button();
    echo "(int)new Button(): pr_int = >" . $this->pr_int . "<\n";

//    $this->pr_int = get_resource();		// resource is not compatible with
    $this->pr_int = (int)get_resource();
    echo "(int)resource: pr_int = >" . $this->pr_int . "<\n";

    echo "\n================ setting pr_float ===================\n\n";

//    $this->pr_float = false;	// bool is incompatible with float
    $this->pr_float = (float)false;	// 0.0
    echo "(float)false: pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = true;		// bool is incompatible with float
    $this->pr_float = (float)true;	// 1.0
    echo "(float)true: pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = 0;		// int is incompatible with float
    $this->pr_float = (float)0;	// 0.0
    echo "(float)0: pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = 100;		// int is incompatible with float
    $this->pr_float = (float)100;	// 100.0
    echo "(float)100: pr_float = >" . $this->pr_float . "<\n";

    $this->pr_float = 0.0;		// 0.0
    $this->pr_float = (float)0.0;	// 0.0
    echo "(float)0.0: pr_float = >" . $this->pr_float . "<\n";

    $this->pr_float = 123.45;	// 123.45
    $this->pr_float = (float)123.45;	// 123.45
    echo "(float)123.45: pr_float = >" . $this->pr_float . "<\n";
    
    $this->pr_float = INF;
    $this->pr_float = (float)INF;
    echo "(float)INF: pr_float = >" . $this->pr_float . "<\n";

    $this->pr_float = NAN;
    $this->pr_float = (float)NAN;
    echo "(float)NAN: pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = "";		// string is incompatible with float
    $this->pr_float = (float)"";	// 0.0
    echo "(float)\"\": pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = "0";		// string is incompatible with float
    $this->pr_float = (float)"0";	// 0.0
    echo "(float)\"0\": pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = "123ABC";	// string is incompatible with float
    $this->pr_float = (float)"123ABC";	// 123.0
    echo "(float)\"123ABC\": pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = "456.78ABC";	// string is incompatible with bool
    $this->pr_float = (float)"456.78ABC";	// 456.78
    echo "(float)\"456.78ABC\": pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = null;		// null is incompatible with float
    $this->pr_float = (float)null;	// 0.0
    echo "(float)null: pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = array();	// array() is incompatible with float
    $this->pr_float = (float)array();	// 0.0
    echo "(float)array(): pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = array(10,20,30);	// array(...) is incompatible with float
    $this->pr_float = (float)array(10,20,30);	// 1.0
    echo "(float)array(10,20,30): pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = new Button();		// Button is incompatible with float
//    $this->pr_float = (float)new Button();
    echo "(float)new Button(): pr_float = >" . $this->pr_float . "<\n";

//    $this->pr_float = get_resource();		// resource is not compatible with float
    $this->pr_float = (float)get_resource();	// 2.0
    echo "(float)resource: pr_float = >" . $this->pr_float . "<\n";

    echo "\n================ setting pr_num ===================\n\n";

//    $this->pr_num = false;		// bool is incompatible with num
//    $this->pr_num = true;		// bool is incompatible with num

    $this->pr_num = 0;
    echo "0: pr_num = >" . $this->pr_num . "<\n";

    $this->pr_num = 100;
    echo "100: pr_num = >" . $this->pr_num . "<\n";

    $this->pr_num = 0.0;
    echo "0.0: pr_num = >" . $this->pr_num . "<\n";

    $this->pr_num = 123.45;
    echo "123.45: pr_num = >" . $this->pr_num . "<\n";

//    $this->pr_num = "";		// string is incompatible with num

//    $this->pr_num = "0";		// string is incompatible with num

//    $this->pr_num = "123ABC";	// string is incompatible with num

//    $this->pr_num = "456.78ABC";	// string is incompatible with bool

//    $this->pr_num = null;		// null is incompatible with num

//    $this->pr_num = array();	// array() is incompatible with num

//    $this->pr_num = array(10,20,30);	// array(...) is incompatible with num

//    $this->pr_num = new Button();		// Button is incompatible with num

//    $this->pr_num = get_resource();		// resource is not compatible with num

    echo "\n================ setting pr_string ===================\n\n";

//    $this->pr_string = (binary)false;	// cast to binary is not permitted

//    $this->pr_string = false;		// bool is incompatible with string
    $this->pr_string = (string)false;	// ""
    echo "(string)false: pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = true;		// bool is incompatible with string
    $this->pr_string = (string)true;	// "1"
    echo "(string)true: pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = 0;
    $this->pr_string = (string)0;	// "0"
    echo "(string)0: pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = 100;
    $this->pr_string = (string)100;	// "100"
    echo "(string)100: pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = 0.0;		// float is incompatible with string
    $this->pr_string = (string)0.0;	// "0"
    echo "(string)0.0: pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = 123.45;		// float is incompatible with string
    $this->pr_string = (string)123.45;	// "123.45"
    echo "(string)123.45: pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = "";		// string is incompatible with string
    $this->pr_string = (string)"";	// ""
    echo "(string)\"\": pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = "0";		// string is incompatible with string
    $this->pr_string = (string)"0";	// "0"
    echo "(string)\"0\": pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = "123ABC";	// string is incompatible with string
    $this->pr_string = (string)"123ABC";	// "123ABC"
    echo "(string)\"123ABC\": pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = "456.78ABC";	// string is incompatible with bool
    $this->pr_string = (string)"456.78ABC";	// "456.78ABC"
    echo "(string)\"456.78ABC\": pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = null;		// null is incompatible with string
    $this->pr_string = (string)null;	// ""
    echo "(string)null: pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = array();	// array() is incompatible with string
//    $this->pr_string = (string)array();
//    echo "(string)array(): pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = array(10,20,30);	// array(...) is incompatible with string
//    $this->pr_string = (string)array(10,20,30);
//    echo "(string)array(10,20,30): pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = new Button();		// Button is incompatible with string
//    $this->pr_string = (string)new Button();
//    echo "(string)new Button(): pr_string = >" . $this->pr_string . "<\n";

//    $this->pr_string = get_resource();		// resource is not compatible with string
    $this->pr_string = (string)get_resource();	// "Resource id #n"
    echo "(string)resource: pr_string = >" . $this->pr_string . "<\n";

    echo "\n================ setting pr_arraykey ===================\n\n";

//    $this->pr_arraykey = false;	// bool is incompatible with arraykey
//    $this->pr_arraykey = true;	// bool is incompatible with arraykey

    $this->pr_arraykey = 0;		// 0
    echo "(arraykey)int: pr_arraykey = >" . $this->pr_arraykey . "<\n";

    $this->pr_arraykey = 100;	// 100
    echo "(arraykey)int: pr_arraykey = >" . $this->pr_arraykey . "<\n";

//    $this->pr_arraykey = 0.0;	// float is incompatible with arraykey

//    $this->pr_arraykey = 123.45;	// float is incompatible with arraykey

    $this->pr_arraykey = "";	// ""
    echo "(arraykey)string: pr_arraykey = >" . $this->pr_arraykey . "<\n";

    $this->pr_arraykey = "0";	// "0"
    echo "(arraykey)string: pr_arraykey = >" . $this->pr_arraykey . "<\n";

    $this->pr_arraykey = "123ABC";	// "123ABC"
    echo "(arraykey)string: pr_arraykey = >" . $this->pr_arraykey . "<\n";

    $this->pr_arraykey = "456.78ABC";	// "456.78ABC"
    echo "(arraykey)string: pr_arraykey = >" . $this->pr_arraykey . "<\n";

//    $this->pr_arraykey = null;		// null is incompatible with arraykey

//    $this->pr_arraykey = array();		// array() is incompatible with arraykey

//    $this->pr_arraykey = array(10,20,30);	// array(...) is incompatible with arraykey

//    $this->pr_arraykey = new Button();	// Button is incompatible with arraykey

//    $this->pr_arraykey = get_resource();	// resource is not compatible with

    echo "\n================ setting pr_resource ===================\n\n";

//    $this->pr_resource = false;		// bool is incompatible with resource 
//    $this->pr_resource = (resource)false;	// cast to resource is not permitted

//    $this->pr_resource = 0;		// int is incompatible with resource
//    $this->pr_resource = 100;	// int is incompatible with resource
//    $this->pr_resource = 0.0;	// float is incompatible with resource
//    $this->pr_resource = 123.45;	// float is incompatible with resource
//    $this->pr_resource = "";	// string is incompatible with resource
//    $this->pr_resource = "0";	// string is incompatible with resource
//    $this->pr_resource = "123ABC";	// string is incompatible with resource
//    $this->pr_resource = "456.78ABC";	// string is incompatible with resource
//    $this->pr_resource = null;	// null is incompatible with resource
//    $this->pr_resource = array();	// array() is incompatible with resource
//    $this->pr_resource = array(10,20,30);// array(...) is incompatible with resource

    $this->pr_resource = get_resource();
    echo "resource: pr_resource = >" . $this->pr_resource . "<\n";

    echo "\n================ setting pr_Button ===================\n\n";

//    $this->pr_Button = false;		// bool is incompatible with Button 
//    $this->pr_Button = (Button)false;	// cast to Button is not permitted
//    $this->pr_Button = (object)false;	// cast to object is not permitted

//    $this->pr_Button = 0;		// int is incompatible with Button
//    $this->pr_Button = 100;		// int is incompatible with Button
//    $this->pr_Button = 0.0;		// float is incompatible with Button
//    $this->pr_Button = 123.45;	// float is incompatible with Button
//    $this->pr_Button = "";		// string is incompatible with Button
//    $this->pr_Button = "0";		// string is incompatible with Button
//    $this->pr_Button = "123ABC";	// string is incompatible with Button
//    $this->pr_Button = "456.78ABC";	// string is incompatible with Button
//    $this->pr_Button = null;	// null is incompatible with Button
//    $this->pr_Button = array();	// array() is incompatible with Button
//    $this->pr_Button = array(10,20,30);// array(...) is incompatible with Button

    $this->pr_Button = new Button("STOP"); 
    echo "new Button(\"STOP\"): pr_Button = >" . $this->pr_Button . "<\n";

    $this->pr_Button = new CustomButton("RAISE"); 
    echo "new CustomButton(\"RAISE\"): pr_Button = >" . $this->pr_Button . "<\n";

//    $this->pr_Button = get_resource();		// resource is not compatible with Button
    echo "resource: pr_Button = >" . $this->pr_Button . "<\n";

    echo "\n================ setting pr_CustomButton===================\n\n";

    $this->pr_CustomButton = new CustomButton("LOWER"); 
    echo "new CustomButton(\"LOWER\"): pr_CustomButton= >" . $this->pr_CustomButton. "<\n";

//    $this->pr_CustomButton = get_resource();	// resource is not compatible with CustomButton 
    echo "resource: pr_CustomButton = >" . $this->pr_CustomButton . "<\n";

    echo "\n================ setting pr_MyCollection ===================\n\n";

//    $this->pr_MyCollection = false;		// bool is incompatible with MyCollection 
//    $this->pr_MyCollection = (MyCollection)false;	// cast to MyCollection is not permitted

    $this->pr_MyCollection = new MyList(); 
    echo "new MyList(): pr_MyCollection = >" . $this->pr_MyCollection . "<\n";

    $this->pr_MyCollection = new MyQueue(); 
    echo "new MyQueue(): pr_MyCollection = >" . $this->pr_MyCollection . "<\n";

//    $this->pr_MyCollection = get_resource();	// resource is not compatible with MyCollection 
//    echo "resource: pr_MyCollection = >" . $this->pr_MyCollection . "<\n";

    echo "\n================ converting non-nullable to corresponding nullable ===================\n\n";

    echo "bool: pr_nbool = >" . $this->pr_nbool . "<\n";
//    $this->pr_nbool = (?bool)true;		// can't cast to any nullable type
    $this->pr_nbool = true;
    echo "bool: pr_nbool = >" . $this->pr_nbool . "<\n";
    $this->pr_nbool = false;
    echo "bool: pr_nbool = >" . $this->pr_nbool . "<\n";
    $this->pr_nbool = $this->pr_bool;
    echo "bool: pr_nbool = >" . $this->pr_nbool . "<\n";

    echo "int: pr_nint = >" . $this->pr_nint . "<\n";
    $this->pr_nint = 100;
    echo "int: pr_nint = >" . $this->pr_nint . "<\n";
    $this->pr_nint = $this->pr_int;
    echo "int: pr_nint = >" . $this->pr_nint . "<\n";

    echo "float: pr_nfloat = >" . $this->pr_nfloat . "<\n";
    $this->pr_nfloat = 123.45;
    echo "float: pr_nfloat = >" . $this->pr_nfloat . "<\n";
    $this->pr_nfloat = $this->pr_float;
    echo "float: pr_nfloat = >" . $this->pr_nfloat . "<\n";

    echo "num: pr_nnum = >" . $this->pr_nnum . "<\n";
    $this->pr_nnum = 100;
    echo "num: pr_nnum = >" . $this->pr_nnum . "<\n";
    $this->pr_nnum = 123.45;
    echo "num: pr_nnum = >" . $this->pr_nnum . "<\n";
    $this->pr_nnum = $this->pr_num;
    echo "num: pr_nnum = >" . $this->pr_nnum . "<\n";

    echo "string: pr_nstring = >" . $this->pr_nstring . "<\n";
    $this->pr_nstring = "123ABC";
    echo "string: pr_nstring = >" . $this->pr_nstring . "<\n";
    $this->pr_nstring = $this->pr_string;
    echo "string: pr_nstring = >" . $this->pr_nstring . "<\n";

    echo "resource: pr_nresource = >" . $this->pr_nresource . "<\n";
    $this->pr_nresource = get_resource();
    echo "resource: pr_nresource = >" . $this->pr_nresource . "<\n";
    $this->pr_nresource = $this->pr_resource;
    echo "resource: pr_nresource = >" . $this->pr_nresource . "<\n";

    echo "Button: pr_nButton = >" . $this->pr_nButton . "<\n";
    $this->pr_nButton = new Button("HOT");
    echo "Button: pr_nButton = >" . $this->pr_nButton . "<\n";
    $this->pr_nButton = new CustomButton("COLD");
    echo "CustomButton: pr_nButton = >" . $this->pr_nButton . "<\n";
    $this->pr_nButton = $this->pr_Button;
    echo "Button: pr_nButton = >" . $this->pr_nButton . "<\n";

    echo "CustomButton: pr_nCustomButton = >" . $this->pr_nCustomButton . "<\n";
    $this->pr_nCustomButton = new CustomButton("ALARM");
    echo "CustomButton: pr_nCustomButton = >" . $this->pr_nCustomButton . "<\n";
    $this->pr_nCustomButton = $this->pr_CustomButton;
    echo "CustomButton: pr_nCustomButton = >" . $this->pr_nCustomButton . "<\n";

//    echo "MyCollection: pr_nMyCollection = >" . $this->pr_nMyCollection . "<\n";
    $this->pr_nMyCollection = new MyList();
    echo "MyCollection: pr_nMyCollection = >" . $this->pr_nMyCollection . "<\n";
    $this->pr_nMyCollection = new MyQueue();
    echo "MyCollection: pr_nMyCollection = >" . $this->pr_nMyCollection . "<\n";
    $this->pr_nMyCollection = $this->pr_MyCollection;

// next line disabaled 'cos of probably bug in that property currently contains a resource when that
// assignment shouldn't have been allowed.
//    echo "MyCollection: pr_nMyCollection = >" . $this->pr_nMyCollection . "<\n";

    echo "\n================ converting one nullable to a different nullable (excluding mixed) ===================\n\n";

    $this->pr_nnum = $this->pr_nint;
    echo "nint: pr_nnum = >" . $this->pr_nnum . "<\n";
    $this->pr_nnum = $this->pr_nfloat;
    echo "nfloat: pr_nnum = >" . $this->pr_nnum . "<\n";
    $this->pr_nButton = $this->pr_nCustomButton;
    echo "nCustomButton: pr_nnum = >" . $this->pr_nnum . "<\n";
//???	$this->pr_nCustomButton = $this->pr_nButton;

    echo "\n================ converting nullable to corresponding non-nullable ===================\n\n";

    $this->pr_nbool = null;
    $this->pr_bool = (bool)$this->pr_nbool;
    echo "pr->bool = >" . $this->pr_bool . "<\n";
    $this->pr_nbool = true;
    $this->pr_bool = (bool)$this->pr_nbool;
    echo "pr->bool = >" . $this->pr_bool . "<\n";

    $this->pr_nint = null;
    $this->pr_int = (int)$this->pr_nint;
    echo "pr->int = >" . $this->pr_int . "<\n";
    $this->pr_nint = 100;
    $this->pr_int = (int)$this->pr_nint;
    echo "pr->int = >" . $this->pr_int . "<\n";

    $this->pr_nfloat = null;
    $this->pr_float = (float)$this->pr_nfloat;
    echo "pr->float = >" . $this->pr_float . "<\n";
    $this->pr_nfloat = 123.45;
    $this->pr_float = (float)$this->pr_nfloat;
    echo "pr->float = >" . $this->pr_float . "<\n";

    $this->pr_nstring = null;
    $this->pr_string = (string)$this->pr_nstring;
    echo "pr->string = >" . $this->pr_string . "<\n";
    $this->pr_nstring = "123ABC";
    $this->pr_string = (string)$this->pr_nstring;
    echo "pr->string = >" . $this->pr_string . "<\n";

    echo "\n================ setting pr_mixed ===================\n\n";

    $this->pr_mixed = false; 
//    $this->pr_mixed = (mixed)false;	// cast to mixed is not permitted
    echo "false: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_bool; 
    echo "pr_bool: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_nbool; 
    echo "pr_nbool: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = true;
    echo "true: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = 0;
    echo "0: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_int; 
    echo "pr_int: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_nint; 
    echo "pr_nint: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = 100;
    echo "100: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = 0.0;
    echo "0.0: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_float; 
    echo "pr_float: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_nfloat; 
    echo "pr_nfloat: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = 123.45;
    echo "123.45: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = "";
    echo "\"\": pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_string; 
    echo "pr_string: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_nstring; 
    echo "pr_nstring: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = "0";
    echo "\"0\": pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = "123ABC";
    echo "\"123ABC\": pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = "456.78ABC";
    echo "\"456.78ABC\": pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = null;
    echo "null: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = array();
    echo "array(): pr_mixed = >" . ((string)$this->pr_mixed) . "<\n";

    $this->pr_mixed = array(10,20,30);
    echo "array(10,20,30): pr_mixed = >" . ((string)$this->pr_mixed) . "<\n";

    $this->pr_mixed = new Button();
    echo "new Button(): pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_Button; 
    echo "pr_Button: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_nButton; 
    echo "pr_nButton: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = new CustomButton();
    echo "new CustomButton(): pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_CustomButton; 
    echo "pr_CustomButton: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_nCustomButton; 
    echo "pr_nCustomButton: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = new MyList();
    echo "new MyList(): pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = new MyQueue();
    echo "new MyQueue(): pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_MyCollection; 

// next line disabled 'cos of probably bug in that property currently contains a resource when that
// assignment shouldn't have been allowed.
//    echo "pr_MyCollection: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = $this->pr_nMyCollection; 

// next line disabled 'cos of probably bug in that property currently contains a resource when that
// assignment shouldn't have been allowed.
//    echo "pr_nMyCollection: pr_mixed = >" . $this->pr_mixed . "<\n";

    $this->pr_mixed = get_resource();
    echo "resource: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_resource; 
    echo "pr_resource: pr_mixed = >" . $this->pr_mixed . "<\n";
    $this->pr_mixed = $this->pr_nresource; 
    echo "pr_nresource: pr_mixed = >" . $this->pr_mixed . "<\n";
  }
}

function main(): void {
    $c1 = new C1();
}

/* HH_FIXME[1002] call to main in strict*/
main();
