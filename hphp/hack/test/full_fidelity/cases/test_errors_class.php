<?hh
final abstract class C {} // no error
abstract final class C {} // no error
final class C {} // no error
abstract class C {} // no error
class C {} // no error
final final class C {} // error
abstract abstract class C {} // error
abstract final abstract class C {} // error
