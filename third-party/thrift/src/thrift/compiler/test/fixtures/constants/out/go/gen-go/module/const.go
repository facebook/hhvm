// Autogenerated by Thrift for thrift/compiler/test/fixtures/constants/src/module.thrift
//
// DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
//  @generated

package module

import (
    thrift "github.com/facebook/fbthrift/thrift/lib/go/thrift/types"
)

// (needed to ensure safety because of naive import list construction)
var _ = thrift.VOID

var GoUnusedProtection__ int

const MyInt int32 = 1337
const Name string = "Mark Zuckerberg"
const MultiLineString string = "This\nis a\nmulti line string.\n"
var States []map[string]int32 = []map[string]int32{
    map[string]int32{
    "San Diego": 3211000,
    "Sacramento": 479600,
    "SF": 837400,
},
    map[string]int32{
    "New York": 8406000,
    "Albany": 98400,
},
}
const X float64 = 1
const Y float64 = 1000000.0
const Z float64 = 1000000000
const ZeroDoubleValue float64 = 0
const LongDoubleValue float64 = 2.59961000990301e-05
var Bin []byte = []byte("a z")
const MyCompany MyCompany = Company_FACEBOOK
const Foo MyStringIdentifier = "foo"
const Bar MyIntIdentifier = 42
var Mymap MyMapIdentifier = map[string]string{
    "keys": "values",
}
var MyApps []Company = []Company{
    Company_FACEBOOK,
    Company___FRIEND__FEED,
}
var Instagram *Internship = NewInternship().
    SetWeeksNonCompat(12).
    SetTitleNonCompat("Software Engineer").
    SetEmployerNonCompat(Company_INSTAGRAM).
    SetCompensationNonCompat(1200).
    SetSchoolNonCompat("Monters University")
var PartialConst *Internship = NewInternship().
    SetWeeksNonCompat(8).
    SetTitleNonCompat("Some Job")
var KRanges []*Range = []*Range{
    NewRange().
    SetMinNonCompat(1).
    SetMaxNonCompat(2),
    NewRange().
    SetMinNonCompat(5).
    SetMaxNonCompat(6),
}
var InternList []*Internship = []*Internship{
    Instagram,
    NewInternship().
    SetWeeksNonCompat(10).
    SetTitleNonCompat("Sales Intern").
    SetEmployerNonCompat(Company_FACEBOOK).
    SetCompensationNonCompat(1000),
}
var Pod_0 *Struct1 = NewStruct1()
var PodS_0 *Struct1 = NewStruct1()
var Pod_1 *Struct1 = NewStruct1().
    SetANonCompat(10).
    SetBNonCompat("foo")
var PodS_1 *Struct1 = NewStruct1().
    SetANonCompat(10).
    SetBNonCompat("foo")
var Pod_2 *Struct2 = NewStruct2().
    SetANonCompat(98).
    SetBNonCompat("gaz").
    SetCNonCompat(
        NewStruct1().
    SetANonCompat(12).
    SetBNonCompat("bar"),
    ).
    SetDNonCompat(
        []int32{
    11,
    22,
    33,
},
    )
var PodTrailingCommas *Struct2 = NewStruct2().
    SetANonCompat(98).
    SetBNonCompat("gaz").
    SetCNonCompat(
        NewStruct1().
    SetANonCompat(12).
    SetBNonCompat("bar"),
    ).
    SetDNonCompat(
        []int32{
    11,
    22,
    33,
},
    )
var PodS_2 *Struct2 = NewStruct2().
    SetANonCompat(98).
    SetBNonCompat("gaz").
    SetCNonCompat(
        NewStruct1().
    SetANonCompat(12).
    SetBNonCompat("bar"),
    ).
    SetDNonCompat(
        []int32{
    11,
    22,
    33,
},
    )
var Pod_3 *Struct3 = NewStruct3().
    SetANonCompat("abc").
    SetBNonCompat(456).
    SetCNonCompat(
        NewStruct2().
    SetANonCompat(888).
    SetCNonCompat(
        NewStruct1().
    SetBNonCompat("gaz"),
    ).
    SetDNonCompat(
        []int32{
    1,
    2,
    3,
},
    ),
    )
var PodS_3 *Struct3 = NewStruct3().
    SetANonCompat("abc").
    SetBNonCompat(456).
    SetCNonCompat(
        NewStruct2().
    SetANonCompat(888).
    SetCNonCompat(
        NewStruct1().
    SetBNonCompat("gaz"),
    ).
    SetDNonCompat(
        []int32{
    1,
    2,
    3,
},
    ),
    )
var Pod_4 *Struct4 = NewStruct4().
    SetANonCompat(1234).
    SetBNonCompat(0.333).
    SetCNonCompat(25)
var U_1_1 *Union1 = NewUnion1().
    SetINonCompat(97)
var U_1_2 *Union1 = NewUnion1().
    SetDNonCompat(5.6)
var U_1_3 *Union1 = NewUnion1()
var U_2_1 *Union2 = NewUnion2().
    SetINonCompat(51)
var U_2_2 *Union2 = NewUnion2().
    SetDNonCompat(6.7)
var U_2_3 *Union2 = NewUnion2().
    SetSNonCompat(
        NewStruct1().
    SetANonCompat(8).
    SetBNonCompat("abacabb"),
    )
var U_2_4 *Union2 = NewUnion2().
    SetUNonCompat(
        NewUnion1().
    SetINonCompat(43),
    )
var U_2_5 *Union2 = NewUnion2().
    SetUNonCompat(
        NewUnion1().
    SetDNonCompat(9.8),
    )
var U_2_6 *Union2 = NewUnion2().
    SetUNonCompat(
        NewUnion1(),
    )
const Apostrophe string = "'"
const TripleApostrophe string = "'''"
const QuotationMark string = "\""
const Backslash string = "\\"
const EscapedA string = "a"
var Char2ascii map[string]int32 = map[string]int32{
    "'": 39,
    "\"": 34,
    "\\": 92,
    "a": 97,
}
var EscapedStrings []string = []string{
    "",
    "",
    " ",
    "'",
    "\"",
    "\n",
    "\r",
    "\t",
    "a",
    "«",
    "j",
    "¦",
    "ayyy",
    "«yyy",
    "jyyy",
    "¦yyy",
    "zzza",
    "zzz«",
    "zzzj",
    "zzz¦",
    "zzzayyy",
    "zzz«yyy",
    "zzzjyyy",
    "zzz¦yyy",
}
var UnicodeList []string = []string{
    "Bulgaria",
    "Benin",
    "Saint Barthélemy",
}
const FalseC bool = false
const TrueC bool = true
const ZeroByte int8 = 0
const Zero16 int16 = 0
const Zero32 int32 = 0
const Zero64 int64 = 0
const ZeroDotZero float64 = 0
const EmptyString string = ""
var EmptyIntList []int32 = []int32{
}
var EmptyStringList []string = []string{
}
var EmptyIntSet []int32 = []int32{
}
var EmptyStringSet []string = []string{
}
var EmptyIntIntMap map[int32]int32 = map[int32]int32{
}
var EmptyIntStringMap map[int32]string = map[int32]string{
}
var EmptyStringIntMap map[string]int32 = map[string]int32{
}
var EmptyStringStringMap map[string]string = map[string]string{
}
var UnicodeMap map[string]string = map[string]string{
    "BG": "Bulgaria",
    "BH": "Bahrain",
    "BÉ": "Saint Barthélemy",
}
const MaxIntDec int64 = 9223372036854775807
const MaxIntOct int64 = 9223372036854775807
const MaxIntHex int64 = 9223372036854775807
const MaxIntBin int64 = 9223372036854775807
const MaxDub float64 = 1.7976931348623157e+308
const MinDub float64 = 2.2250738585072014e-308
const MinSDub float64 = 5e-324
const MaxPIntDec int64 = 9223372036854775807
const MaxPIntOct int64 = 9223372036854775807
const MaxPIntHex int64 = 9223372036854775807
const MaxPIntBin int64 = 9223372036854775807
const MaxPDub float64 = 1.7976931348623157e+308
const MinPDub float64 = 2.2250738585072014e-308
const MinPSDub float64 = 5e-324
const MinIntDec int64 = -9223372036854775808
const MinIntOct int64 = -9223372036854775808
const MinIntHex int64 = -9223372036854775808
const MinIntBin int64 = -9223372036854775808
const MaxNDub float64 = -1.7976931348623157e+308
const MinNDub float64 = -2.2250738585072014e-308
const MinNSDub float64 = -5e-324
var I2B map[int32]bool = map[int32]bool{
    0: false,
    1: true,
    2: true,
    3: false,
}
var I2B_REF map[int32]bool = map[int32]bool{
    0: false,
    1: true,
    2: true,
    3: false,
}
