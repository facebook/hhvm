<?hh

  function test () {
    //Ttuples
    new least_upper_bound<(int, string), (float, string)>();
    new least_upper_bound<(int, string), (num, string)>();
    new least_upper_bound<(int, string, int), (num, int)>();
    //Tarraykind
    new least_upper_bound<array<(int, int)>, array<(num, int)>>();
    new least_upper_bound<array<(int, int, int)>, array<(num, int)>>();
    //Tclass
    new least_upper_bound<classid<(int, int)>, classid<(num, int)>>();
    new least_upper_bound<classid1<(int,int)>, classid2<(num, int)>>();
    new least_upper_bound<classid<int>, classid<(num, int)>>();
    new least_upper_bound<classid1<int>, classid2<(num, int)>>();
    //Toption
    new least_upper_bound<(int, ?string), (?num, string)>();
    new least_upper_bound<?int, ?(string, float)>();
    new least_upper_bound<int, ?float>();
    new least_upper_bound<?int, ?float, num>();
}
