__StarlarkEnum__ = record(name=str.type, value=int.type)
__EmptyEnum_enum__ = record(
)

EmptyEnum = __EmptyEnum_enum__(
)

__City_enum__ = record(
    NYC = __StarlarkEnum__.type,
    MPK = __StarlarkEnum__.type,
    SEA = __StarlarkEnum__.type,
    LON = __StarlarkEnum__.type,
)

City = __City_enum__(
    NYC = __StarlarkEnum__(name="NYC", value=0),
    MPK = __StarlarkEnum__(name="MPK", value=1),
    SEA = __StarlarkEnum__(name="SEA", value=2),
    LON = __StarlarkEnum__(name="LON", value=3),
)

__Company_enum__ = record(
    FACEBOOK = __StarlarkEnum__.type,
    WHATSAPP = __StarlarkEnum__.type,
    OCULUS = __StarlarkEnum__.type,
    INSTAGRAM = __StarlarkEnum__.type,
)

Company = __Company_enum__(
    FACEBOOK = __StarlarkEnum__(name="FACEBOOK", value=0),
    WHATSAPP = __StarlarkEnum__(name="WHATSAPP", value=1),
    OCULUS = __StarlarkEnum__(name="OCULUS", value=2),
    INSTAGRAM = __StarlarkEnum__(name="INSTAGRAM", value=3),
)

myInt = 1337

name = "Mark Zuckerberg"

multi_line_string = "This\nis a\nmulti line string.\n"

states = [
  {
  "San Diego": 3211000,
  "Sacramento": 479600,
  "SF": 837400,
},
  {
  "New York": 8406000,
  "Albany": 98400,
},
]

x = 1

y = 1000000.0

z = 1000000000

zeroDoubleValue = 0

longDoubleValue = 2.59961000990301e-05

my_company = Company.FACEBOOK

foo = "foo"

bar = 42

mymap = {
  "keys": "values",
}

apostrophe = "'"

tripleApostrophe = "'''"

quotationMark = "\""

backslash = "\\"

escaped_a = "a"

char2ascii = {
  "'": 39,
  "\"": 34,
  "\\": 92,
  "a": 97,
}

escaped_strings = [
  "\001",
  "\037",
  " ",
  "'",
  "\"",
  "\n",
  "\r",
  "\011",
  "a",
  "\302\253",
  "j",
  "\302\246",
  "ayyy",
  "\302\253yyy",
  "jyyy",
  "\302\246yyy",
  "zzza",
  "zzz\302\253",
  "zzzj",
  "zzz\302\246",
  "zzzayyy",
  "zzz\302\253yyy",
  "zzzjyyy",
  "zzz\302\246yyy",
]

false_c = False

true_c = True

zero_byte = 0

zero16 = 0

zero32 = 0

zero64 = 0

zero_dot_zero = 0

empty_string = ""

empty_int_list = [
]

empty_string_list = [
]

empty_int_int_map = {
}

empty_int_string_map = {
}

empty_string_int_map = {
}

empty_string_string_map = {
}

maxIntDec = 9223372036854775807

maxIntOct = 9223372036854775807

maxIntHex = 9223372036854775807

maxIntBin = 9223372036854775807

maxDub = 1.7976931348623157e+308

minDub = 2.2250738585072014e-308

minSDub = 5e-324

maxPIntDec = 9223372036854775807

maxPIntOct = 9223372036854775807

maxPIntHex = 9223372036854775807

maxPIntBin = 9223372036854775807

maxPDub = 1.7976931348623157e+308

minPDub = 2.2250738585072014e-308

minPSDub = 5e-324

minIntDec = -9223372036854775808

minIntOct = -9223372036854775808

minIntHex = -9223372036854775808

minIntBin = -9223372036854775808

maxNDub = -1.7976931348623157e+308

minNDub = -2.2250738585072014e-308

minNSDub = -5e-324

I2B = {
  0: False,
  1: True,
  2: True,
  3: False,
}

I2B_REF = {
  0: False,
  1: True,
  2: True,
  3: False,
}

