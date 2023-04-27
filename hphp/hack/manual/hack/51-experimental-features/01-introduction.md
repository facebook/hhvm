The following pages of documentation describe Hack language features in the *experimental* phase.

## About Experimental Features
We have documented these features as they may appear in built-ins, including the Hack Standard Library.

We _do not_ recommend using any of these features until they are formally announced and integrated into the main documentation set.

## Enabling an Experimental Feature
To use an experimental feature, add the `__EnableUnstableFeatures` file attribute to any files containing that feature.

```Hack no-extract
<<file:__EnableUnstableFeatures('experimental_feature_name')>>
```

You can also specify multiple features:

```Hack no-extract
<<file:__EnableUnstableFeatures('experimental_feature_name', 'other_experimental_feature_name')>>
```
