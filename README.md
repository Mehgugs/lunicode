## Lunicode 🌜

Lua bindings to utf8proc.

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

- *integer (codepoint)* `codepoint`

#### *string (unicode category)* `category_string(codepoint)`

Gets the two character Unicode category signifier for the given codepoint.

- *integer (codepoint)* `codepoint`

#### *boolean, integer (state)* `grapheme_break(codepoint_1, codepoint_2, state)`

Given a pair of consecutive codepoints, return whether a grapheme break is permitted between them (as defined by the extended grapheme clusters in UAX#29).
See below for an example of how to use this function.

- *integer (codepoint)* `codepoint_1`
- *integer (codepoint)* `codepoint_2` The codepoint adjacent to `codepoint_1` in the string.

- *integer* `state` Due to Unicode 9.0.0, this algorithm requires
  state to break graphemes. This state can be passed in as an integer which
  should be initialized to 0.

#### *table (utf8proc_property_t)* `properties(codepoint)`

Returns a table containing the fields of the given codepoint's `utf8proc_property_t` struct.

- *integer (codepoint)* `codepoint`

#### *integer|boolean* `property(codepoint, field)`

Returns a field from the given codepoint's `utf8proc_property_t` struct.

- *integer (codepoint)* `codepoint`
- *string* `field?` A field you wish to select, if you do not provide a field
  the function returns the category field.


### Iterating over the graphemes in a string:

This example can probably be tuned into a much more efficient procedure but this
is written as such to illustrate the principle.

```lua

local wrap  = coroutine.wrap
local yield = coroutine.yield

local insert = table.insert
local unpack = table.unpack

local codes = utf8.codes
local char  = utf8.char

local lunicode = require"lunicode"

local grapheme_break = lunicode.grapheme_break


local graphemes do
  local function graphemes_iterator(string)
    local collection = {}
    local n, state = 0, 0
    local broken
    for position, code in codes(string) do
      if position == 1 then goto continue end

      broken, state = grapheme_break(collection[n], code, state)

      if broken then
        yield(char(unpack(collection, 1, n)))
        collection = {}
        n = 0
      end

      ::continue::
      insert(collection, code)
      n = n + 1
    end
  end

  function graphemes(string) return wrap(graphemes_iterator), string end
end

-- The above function should illustrate how to use the information returned by grapheme_break, when given codepoints and the state.

-- For those curious, a 'cuter' way to achieve an iterator would be the following code below:

local function graphemes_cute(str)
    return lunicode.map(str, {CHARBOUND = true}):gmatch("[^\xFF]+")
end

```
