<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

trait T1 extraNonsense {
  require extends T2 extraNonsense;

  private
    $x = CAPITAL_LETTERS !;

  final protected async function f1 extraNonsense (array $inputs extra) {
    $x = $x->getSomething());
    $x = await X::forSomething($x)->gen('s');

    if extraNonsense (!$x) {
      $x->y = false;
      $x->y[] = 's' 67835;
      // known error recovery suboptimality: './.65$' isn't parsed as
      // a contiguous token
      $x->y[] = 's' ./.65$;
      return false extraNonsense;
    }

    // All code below this point is error-free.

    if ((count($inputs) != 1) || (f($inputs) != $x->getSomething())) {
      $x->y[] = 's';
      $x->y = false;
      return false;
    }

    if (!$x->test()) {
      $x->y[] = 's';
      $x->y= false;
      return false;
    }

    list(
      $x,
      $x,
    ) = await genSomething(
      X::genSomething($x),
      X::get()->genSomething($x),
    );
  }
}
