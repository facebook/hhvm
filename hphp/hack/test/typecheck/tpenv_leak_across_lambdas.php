<?hh

abstract class RepFieldSettings {
  abstract const type TValue;
}

interface IRepField<+TSettings as RepFieldSettings> {}

class RepIntSettings extends RepFieldSettings {
  const type TValue = int;
}

class RepIntField implements IRepField<RepIntSettings> {}

enum class RepAccessors: IRepField<RepFieldSettings> {
  RepIntField A = new RepIntField();
  RepIntField B = new RepIntField();
  RepIntField C = new RepIntField();
  RepIntField D = new RepIntField();
  RepIntField E = new RepIntField();
}

interface IRepBuilderMethods<+TChainableBuilder as IRepBuilder> {
  public function setField<TValue>(
    HH\EnumClass\Label<RepAccessors, IRepField<
      RepFieldSettings with { type TValue super TValue },
    >> $label,
    TValue $value,
  ): TChainableBuilder;
}

interface IRepBuilder extends IRepBuilderMethods<this::TChainableBuilder> {
  abstract const type TChainableBuilder as
    IRepBuilder with { type TChainableBuilder = this::TChainableBuilder };
}

function tpenv_leak_repro(): dict<string, (function(IRepBuilder): void)> {
  $x = dict[
    'a' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
    'b' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
    'c' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
    'd' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
    'e' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
    'f' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
    'g' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
    'h' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
    'i' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
    'j' => (IRepBuilder $b) ==> {
      $b->setField(#A, 1)->setField(#B, 2)->setField(#C, 3)->setField(#D, 4)->setField(#E, 5);
    },
  ];
  return $x;
}
