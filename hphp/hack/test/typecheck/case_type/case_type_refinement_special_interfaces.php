<?hh
<<file:__EnableUnstableFeatures('case_types')>>

case type StringishTest = resource | string | StringishObject;

function test_stringish(StringishTest $x): ?resource {
    if ($x is Stringish) {
        return null;
    }
    return $x;
}

abstract class :xhp implements XHPChild {}

case type XHPChildTest = resource | int | string | vec<int> | :xhp;

function test_xhpchild(XHPChildTest $x): ?resource {
    if ($x is XHPChild) {
        return null;
    }
    return $x;
}

case type ContainerTest =
  resource | Vector<int> | keyset<int> | vec<int> | dict<int, int>;

function test_container(ContainerTest $x): ?resource {
    if ($x is Container<_>) {
        return null;
    }
    return $x;
}

function test_keyedcontainer(ContainerTest $x): ?resource {
    if ($x is KeyedContainer<_, _>) {
        return null;
    }
    return $x;
}

function test_traversable(ContainerTest $x): ?resource {
    if ($x is Traversable<_>) {
        return null;
    }
    return $x;
}

function test_keyedtraversable(ContainerTest $x): ?resource {
    if ($x is KeyedTraversable<_, _>) {
        return null;
    }
    return $x;
}
