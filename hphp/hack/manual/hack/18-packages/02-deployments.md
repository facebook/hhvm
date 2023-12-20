Once you've defined a set of packages, you can deploy them using **deployments**.

## Deployments
A **deployment** is defined as a set of packages. Deployments are also defined in PACKAGES.toml:

```toml PACKAGES.toml
[packages]

[packages.production]
uses=["prod.*", "my_prod"] # Package contains all modules that start with `prod`, and the module "my_prod".

[packages.test]
uses=["test.*"]
includes=["production"] # Package depends on the production package

[deployments]
[deployments.production]
packages=["production"]

[deployments.test]
packages=["test", "production"] # Since the test package includes production, they must be deployed together.
```

When building in [repo authoritative](../../hhvm/advanced-usage/repo-authoritative.md) mode, you can pass in the build configuration `Eval.ActiveDeployment = <deployment>` to set the active deployment. HHVM will then include only the files in the active deployment when building.

## Deployment domains
In CLI-server mode, HHVM can direct different domains to different deployments, allowing you to treat a web request as if it were built in repo mode with a specific deployment. You can set the `domains` value of to any deployment to a list of regexes:

```toml
[deployments.production]
packages=["production"]
domains=["^.*my_website\.com"]
```
The domains field matches on the `Host` field of any web request that HHVM serves. In this example, traffic that hits `my_website.com` will be treated as if the active deployment were production. Note that a hostname can match multiple regexes, but the **first** deployment listed which has a regex that matches the hostname will be used.
