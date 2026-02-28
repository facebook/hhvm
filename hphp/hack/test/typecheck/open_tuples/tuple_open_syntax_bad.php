<?hh



// Illegal tuple types
type T1 = (int,optional string,bool);
type T2 = (int, optional mixed...);
type T3 = (optional bool, arraykey..., int...);
type T4 = (int...,bool);
type T5 = (int...,optional string);
