# Features

<!-- Marks a feature as supported in a language. -->
export const Supported = () => (<span class="feature-supported">✓</span>);

<!-- Marks a feature as partially supported in a language. -->
export const Partial = ({children = "Partial"}) => (
  <span class="feature-partial">{children}</span>
);

<!-- Marks a feature as in progress (being developed) in a language. -->
export const InProgress = () => (
  <span class="feature-partial">In progress</span>
);

<!-- Marks a feature as unsupported in a language. -->
export const Unsupported = () => (<span class="feature-unsupported">✗</span>);

The following tables document the current level of support for each Thrift
feature in different languages.

| Thrift feature | C++ | Hack | Java | Python |
| :------------- | :-: | :--: | :--: | :----: |
| [Serialization](/features/serialization/index.md) | <Supported/> | <Supported/> | <Supported/> | <Supported/> |
| [Universal names](/features/universal-name.md) | <Supported/> | <Supported/> | <Supported/> | <Supported/> |
| [Streaming](/fb/features/streaming/index.md) | <Supported/> | <Partial>Client[^1]</Partial> | <Partial>Client[^1]</Partial> | <Supported/> |
| [Interactions](/fb/features/interactions.md) | <Supported/> | <Partial>Client[^1]</Partial> | <Partial>Client[^1]</Partial> | <Partial>Client[^1]</Partial> |
| [Adapters](/features/adapters.md) | <Supported/> | <InProgress/> | <Supported/> | <Supported/> |
| [Dynamic values (Any)](/features/any.md) | <InProgress/> | <Unsupported/> | <Unsupported/> | <Unsupported/> |
| [Terse write](/features/terse-write.md) | <Supported/> | <Supported/> | <Supported/> | <Supported/> |

[^1]: You can write Thrift clients but not servers using this feature. These
clients can connect to servers written in other languages.

Experimental features:

| Thrift feature | C++ | Hack | Java | Python |
| :------------- | :-: | :--: | :--: | :----: |
| [Patch](/features/patch/patch.md) | <InProgress/> | <Unsupported/> | <Unsupported/> | <Unsupported/> |
| [Deterministic Hashing](/fb/languages/cpp/hash.md) | <Supported/> | <Unsupported/> | <Unsupported/> | <Unsupported/> |

The level of support of a feature in a given language is defined as follows:

| Symbol | Meaning |
| :----: | :------ |
| <Supported/> | Fully supported by Thrift |
| <InProgress/> | Under development |
| <Partial>&lt;status&gt;</Partial> | Partially supported with the given status |
| <Unsupported/> | Not supported |
