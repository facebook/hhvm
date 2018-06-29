<?hh // strict

// A ppl class cannot only extend a final ppl base class

<<__PPL>>
final class BaseClass {
}

<<__PPL>>
class ChildClass extends BaseClass {
}
