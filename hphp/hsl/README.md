[![Build Status](https://travis-ci.org/hhvm/hsl.svg?branch=master)](https://travis-ci.org/hhvm/hsl)

# Hack Standard Library

The goal of the Hack Standard Library is to provide a consistent, centralized,
well-typed set of APIs for Hack programmers. We aim to achieve this by
implementing the library according to codified design principles.

This library is especially useful for working with the Hack arrays (`vec`,
`keyset`, and `dict`).

For future APIs, see
[the experimental repository](https://github.com/hhvm/hsl-experimental).

## Status of this repository

As of 2021-05-04, the HSL is moving to be built into HHVM and the typechecker;
as such, this repository currently only contains tests.

In the near future, we expect this repository to either be archived, or to
become a partial mirror of the HHVM repository.

The HSL itself remains supported - this is just moving where the code lives.

## Contributing

Contributions should be made to the `hphp/hsl` subdirectory of
[the HHVM repository](https://github.com/facebook/hhvm/).


## License

The Hack Standard Library is MIT-licensed.
