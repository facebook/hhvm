<?hh

type ID<T> = T;

// TestClass : * -> (* -> *) -> *
class TestClass<T1,T2<T3>> {}


// Kind of Test1 and Test2:
type Test1<TC<TA1,TA2<TA3>>> = TC<int, ID>;
newtype Test2<TC<TA1,TA2<TA3>>> = TC<int, ID>;


function test1(Test1<TestClass> $x ) : void {} // ok
function test2(Test2<TestClass> $x ) : void {} // ok
