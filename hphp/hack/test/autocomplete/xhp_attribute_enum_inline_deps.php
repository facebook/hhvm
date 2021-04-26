<?hh

class :foo:bar {
  attribute enum {"firefly", 1} my-attribute;
}

class :banana:phone {
    attribute :foo:bar;
}

class :poo:bear {
    attribute :banana:phone;
}

function main(): void {
  <poo:bear my-attribute={AUTO332}
}
