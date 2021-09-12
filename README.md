## Lunicode 🌜

Lua bindings to utf8proc.

## You will need to build utf8proc and make it available on your machine!

### More functions will be added as I need them.

### Reference

#### *table* `metadata`

- *string* `utf8proc_version`
  The version of `utf8proc` lunicode has loaded.
- *string* `unicode_version`
  The version of unicode `utf8proc` is using.

#### *boolean* `is_valid(n)`

Checks whether `n` is a valid codepoint using `utf8proc_codepoint_valid`.

#### *string* `map(string, options)`

Performs the utf8proc mapping operation. This decomposes the input with respect to
options provided and then re-encodes it as a utf8 string.

- *string* `string`
- *table (set)* `options`
  This should be a set which can have any of the `utf8proc_option_t` names as keys.

#### *string* `normalize(string, mode)`

Performs a `map` using options suitable for unicode normalization.

- *string* `string`
- *string (normalization mode)* `mode`
  Optionally one of: `NFC, NFD, NFKC, NFKD`. If no argument is passed `NFC` is used.

#### *integer (utf8proc_category_t)* `category(codepoint)`

Gets the Unicode category for the given codepoint.

- *integer (codepoint)* codepoint

#### *string (unicode category)* `category_string(codepoint)`

Gets the two character Unicode category signifier for the given codepoint.

- *integer (codepoint)* codepoint