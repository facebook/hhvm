<?hh

class C<T1, T2> {}

class D<T3> extends C<int, T3> {

}

class E extends D<string> {} // E's linearization will contain C<int, T3>
// We do not calculate substitutions during linearization, but will need to calculate what T3 really means
// at a later step.
