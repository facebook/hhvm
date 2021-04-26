<?hh

class :foo:bar {
  attribute enum {"firefly", 1} my-attribute;
}

class :banana:phone extends :foo:bar {
}

class :poo:bear extends :banana:phone {
}

function main(): void {
  <poo:bear my-attribute={AUTO332}
}
