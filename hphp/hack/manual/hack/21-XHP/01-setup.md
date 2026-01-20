# Setup

XHP provides a native XML-like representation of output.

After adding the required dependencies, read the [Introduction](/hack/XHP/introduction).

## The XHP-Lib Library
While XHP syntax is a part of the Hack language, implementation is in [XHP-Lib](https://github.com/hhvm/xhp-lib/), a library that needs to be installed with [Composer](https://getcomposer.org/).

* XHP-Lib includes the base classes and interfaces, and definitions of standard HTML elements.
* Support for namespaced XHP classes (elements like `<p>`) is enabled by default since HHVM 4.73.

### XHP-Lib Versions
We stongly recommend using XHP-Lib v4, which includes XHP namespace support. XHP-Lib v3 is not officially maintained.

**Important:** All of the guides in this section are written with the assumption that XHP-Lib v4 is used, but there are notes pointing out any major differences—look for the **Historical note** sections.

<FbCaution>

XHP namespaces are not enabled in Facebook's WWW repository, so all *Historical note* sections apply.

</FbCaution>

#### XHP-Lib v4
Used when XHP namespace support is enabled. Declares all base classes, interfaces and elements in namespaces (e.g. standard HTML elements are in `Facebook\XHP\HTML`). It is also more strict (e.g. disallows most mutations after an element is rendered) and deprecates some features (e.g. attribute "clobbering" in `XHPHTMLHelpers`).

To install, add `facebook/xhp-lib` to your `composer.json` manually, or run `composer require facebook/xhp-lib ^4.0`

#### XHP-Lib v3
Used in older HHVM versions or when XHP namespace support is disabled. Declares everything in the root namespace, with the exception of `Facebook\XHP\ChildValidation`.

To install, add `facebook/xhp-lib` to your `composer.json` manually, or run `composer require facebook/xhp-lib ^3.0`

### Enable Namespace Support
We recommend using HHVM 4.73 or newer, since it's more thoroughly tested and doesn't require any extra configuration, however, XHP namespace support can be enabled in older HHVM versions (since around HHVM 4.46) by adding the following flags to your `.hhconfig`:

```
enable_xhp_class_modifier = true
disable_xhp_element_mangling = true
```

And to `hhvm.ini` (or provided via `-d` when executing `hhvm`):

```
hhvm.hack.lang.enable_xhp_class_modifier=true
hhvm.hack.lang.disable_xhp_element_mangling=true
```

### Disable Namespace Support
In HHVM 4.73 or newer, XHP namespace support can be disabled by setting these to `false`.

```
hhvm.hack.lang.enable_xhp_class_modifier=false
hhvm.hack.lang.disable_xhp_element_mangling=false
```

If these flags are disabled, or if you're using an older version of HHVM:

- XHP classes cannot be declared in namespaces (only in the root namespace)
- any code that uses XHP classes also cannot be in a namespace, as HHVM previously didn't have any syntax to reference XHP classes across namespaces
- note that the above two rules are not consistently enforced by the typechecker or the runtime, but violating them can result in various bugs
- it is, however, possible to use namespaced code from inside XHP class declarations

Make sure to also use the correct version of XHP-Lib based on whether XHP namespace support is enabled in your HHVM version.

## HHVM Configuration Flags
These are not enabled by default in any HHVM version, but we recommend enabling them in any new Hack projects:

- `disable_xhp_children_declarations = true` disables the old way of declaring allowed children, which has been deprecated in favor of the `Facebook\XHP\ChildValidation\Validation` trait. See [Children](/hack/XHP/extending#children) for more information.
- `check_xhp_attribute = true` enables the typechecker to check that all required attributes are provided. Otherwise, these would only be errors at runtime.
