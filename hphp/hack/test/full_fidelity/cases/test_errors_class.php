<?hh
final abstract class C {} // no error
abstract final class C {} // no error
final class C {} // no error
abstract class C {} // no error
class C {} // no error
final final class C {} // error ; TODO T22019948
abstract abstract class C {} // error ; TODO T22019948
abstract final abstract class C {} // error ; TODO T22019948
