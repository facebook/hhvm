__EmptyEnum_enum__ = record(
)

EmptyEnum = __EmptyEnum_enum__(
)

__City_enum__ = record(
    NYC = int.type,
    MPK = int.type,
    SEA = int.type,
    LON = int.type,
)

City = __City_enum__(
    NYC = 0,
    MPK = 1,
    SEA = 2,
    LON = 3,
)

__Company_enum__ = record(
    FACEBOOK = int.type,
    WHATSAPP = int.type,
    OCULUS = int.type,
    INSTAGRAM = int.type,
)

Company = __Company_enum__(
    FACEBOOK = 0,
    WHATSAPP = 1,
    OCULUS = 2,
    INSTAGRAM = 3,
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

