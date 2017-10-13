(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
  child : t option;
  start_offset : int;
  end_offset : int;
  message : string
}

val make : ?child:t option -> int -> int -> string -> t

val to_positioned_string : t -> (int -> int * int) -> string

val compare : t -> t -> int

val exactly_equal : t -> t -> bool

val message : t -> string

val error0001 : string
val error0002 : string
val error0003 : string
val error0004 : string
val error0005 : string
val error0006 : string
val error0007 : string
val error0008 : string
val error0009 : string
val error0010 : string
val error0011 : string
val error0012 : string
val error0013 : string
val error0014 : string

(* Syntactic errors *)
val error1001 : string
val error1002 : string
val error1003 : string
val error1004 : string
val error1006 : string
val error1007 : string
val error1008 : string
val error1009 : string
val error1010 : string
val error1011 : string
val error1012 : string
val error1013 : string
val error1014 : string
val error1015 : string
val error1016 : string
val error1017 : string
val error1018 : string
val error1019 : string
val error1020 : string
val error1021 : string
val error1022 : string
val error1023 : string
val error1024 : string
val error1025 : string
val error1026 : string
val error1027 : string
val error1028 : string
val error1029 : string
val error1030 : string
val error1031 : string
val error1032 : string
val error1033 : string
val error1034 : string
val error1035 : string
val error1036 : string
val error1037 : string
val error1038 : string
val error1039 : string
val error1040 : string
val error1041 : string
val error1042 : string
val error1043 : string
val error1044 : string
val error1045 : string
val error1046 : string
val error1047 : string
val error1048 : string
val error1050 : string
val error1051 : string
val error1052 : string
val error1053 : string
val error1054 : string
val error1055 : string
val error1056 : string
val error1057 : string -> string
val error1058 : string -> string -> string

val error2001 : string
val error2002 : string
val error2003 : string
val error2004 : string
val error2005 : string
val error2006 : string
val error2007 : string
val error2008 : string
val error2009 : string
val error2010 : string
val error2011 : string
val error2012 : string
val error2013 : string
val error2014 : string
val error2015 : string
val error2016 : string
val error2017 : string
val error2018 : string
val error2019 : string
val error2020 : string
val error2021 : string
val error2022 : string

val error2029 : string
val error2030 : string
val error2031 : string
val error2032 : string
val error2033 : string
val error2034 : string
val error2035 : string
val error2036 : string
val error2037 : string
val error2038 : string -> string
val error2039 : string -> string -> string -> string
val error2040 : string
val error2041 : string
val error2042 : string
val error2043 : string
val error2044 : string -> string -> string
val error2045 : string
val error2046 : string
val error2047 : string -> string
val error2048 : string
val error2049 : string
val error2050 : string
val error2051 : string
val error2052 : string
val error2053 : string
val error2054 : string
val error2055 : string
val error2056 : string
val error2057 : string
val error2058 : string
val error2059 : string
val error2060 : string
val error2061 : string
val error2062 : string
val error2063 : string
val error2064 : string
val error2065 : string
val error2066 : string
val error2067 : string
val error2068 : string
