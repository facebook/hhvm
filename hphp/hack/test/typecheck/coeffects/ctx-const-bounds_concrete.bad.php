<?hh

class BadWhenBoundsAppearInConcreteClass {
  const ctx Clower super [defaults] = [defaults];
  const ctx Cupper as [defaults] = [defaults];
  const ctx Cboth as [] super [defaults] = [defaults];
}

abstract class BadWhenBoundsAppearInAbstractClass {
  const ctx Clower super [defaults] = [defaults];
  const ctx Cupper as [defaults] = [defaults];
  const ctx Cboth as [] super [defaults] = [defaults];
}

interface BadWhenBoundsAppearInInterface {
  const ctx Clower super [defaults] = [defaults];
  const ctx Cupper as [defaults] = [defaults];
  const ctx Cboth as [] super [defaults] = [defaults];
}
