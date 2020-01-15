<?hh

final class :head {

}

final class ClassThatDefinitelyExists {

}

class :my-html {
  category %phrase;
  children (
    :head,
    :body,
    pcdata,
    any*,
    banana,
    ClassThatDefinitelyExists,
    %other,
    (PCDATA | Undefined+));
}
