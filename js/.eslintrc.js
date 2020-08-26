module.exports = {
  env: {
    commonjs: true,
    es6: true,
    node: true
  },
  extends: [
    'standard'
  ],
  globals: {
    Atomics: 'readonly',
    SharedArrayBuffer: 'readonly'
  },
  parserOptions: {
    ecmaVersion: 11
  },
  rules: {
    'import/prefer-default-export': ['off'],
    'no-console': ['off'],
    'no-underscore-dangle': ['off'],
    'class-methods-use-this': ['off'],
    'no-param-reassign': ['off'],
    'no-restricted-syntax': ['off'],
    'no-prototype-builtins': ['off'],
    'guard-for-in': ['off']
  }
}
