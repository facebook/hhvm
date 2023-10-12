<?hh

class Supertype {}
class Subtype extends Supertype {}

function test(Supertype $supertype, Subtype $subtype, mixed $mixed): void {
}
