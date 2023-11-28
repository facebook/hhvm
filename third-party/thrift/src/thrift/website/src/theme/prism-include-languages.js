import siteConfig from '@generated/docusaurus.config';
export default function prismIncludeLanguages(PrismObject) {
  const {
    themeConfig: {prism},
  } = siteConfig;
  const {additionalLanguages} = prism;
  // Prism components work on the Prism instance on the window, while prism-
  // react-renderer uses its own Prism instance. We temporarily mount the
  // instance onto window, import components to enhance it, then remove it to
  // avoid polluting global namespace.
  // You can mutate PrismObject: registering plugins, deleting languages... As
  // long as you don't re-assign it
  globalThis.Prism = PrismObject;
  additionalLanguages.forEach((lang) => {
    // eslint-disable-next-line global-require, import/no-dynamic-require
    require(`prismjs/components/prism-${lang}`);
  });

  // Syntax highlighting rules for Thrift code:
  Prism.languages.thrift = {
    'keyword': [
      /\b(?:binary|bool|byte|const|cpp_include|double|enum|exception)\b/,
      /\b(?:extends|float|hs_include|i16|i32|i64|include|interaction)\b/,
      /\b(?:list|map|namespace|optional|performs|required|service|set)\b/,
      /\b(?:stream|string|struct|throws|typedef|union|void)\b/,
      // Context-sensitive keywords:
      /\b(?:client|idempotent|oneway|package|permanent|readonly)\b/,
      /\b(?:safe|server|sink|stateful|transient)\b/
    ],
    'boolean': /\b(?:false|true)\b/,
    'number': /\b0x[\da-f]+\b|(?:\b\d+(?:\.\d*)?|\B\.\d+)(?:e[+-]?\d+)?/i,
    'string': {
      pattern: /(?:"[^"]*"|'[^']*')/,
      greedy: true
    },
    'operator': /[+-]/,
    'punctuation': /[,;]/,
    'comment': [
      {
        'pattern': /\/\/.*/,
        'greedy': true
      },
      {
        'pattern': /\/\*(\*(?!\/)|[^*])*\*\//,
        'greedy': true
      }
    ]
  };

  // Syntax highlighting rules for grammar (modified BNF):
  Prism.languages.grammar = {
    'defined-symbol': {
      'pattern': /(^|\n)[a-z][a-z_]*/,
      'lookbehind': true,
      'alias': 'keyword'
    },
    'string': /(?:"[^"]*"|'[^']*')/,
    'operator': /(?:[|*]|\.\.\.|::=)/,
    'punctuation': /[\[\]()]/
  };

  delete globalThis.Prism;
}
