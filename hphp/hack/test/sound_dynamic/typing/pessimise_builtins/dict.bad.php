<?hh

class C {}

function test_dict() : dict<int,C> {
  return dict<int,dynamic>[];
}
