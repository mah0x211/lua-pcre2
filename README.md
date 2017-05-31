# lua-pcre2

PCRE2 bindings for lua.

**NOTE:** this module is under heavy development.


## Dependencies

- luarocks-fetch-gitrec: <https://github.com/siffiejoe/luarocks-fetch-gitrec>
- lauxhlib: <https://github.com/mah0x211/lauxhlib>
- pcre2: <https://github.com/mah0x211/pcre2>

---

## pcre2 module

```lua
local pcre2 = require('pcre2')
```


## Constants


### Compile flags

- `ANCHORED`: Force pattern anchoring.
- `ALT_BSUX`: Alternative handling of `\u`, `\U`, and `\x`.
- `ALT_CIRCUMFLEX`: Alternative handling of `^` in multiline mode.
- `AUTO_CALLOUT`: Compile automatic callouts.
- `CASELESS`: Do caseless matching.
- `DOLLAR_ENDONLY`: `$` not to match newline at end.
- `DOTALL`: `.` matches anything including NL.
- `DUPNAMES`: Allow duplicate names for subpatterns.
- `EXTENDED`: Ignore white space and `#` comments.
- `FIRSTLINE`: Force matching to be before newline.
- `MATCH_UNSET_BACKREF`: Match unset back references.
- `MULTILINE`: `^ and `$` match newlines within data.
- `NEVER_BACKSLASH_C`: Lock out the use of `\C` in patterns.
- `NEVER_UCP`: Lock out `UCP` flag, e.g. via (*UCP)
- `NEVER_UTF`: Lock out `UTF` flag, e.g. via (*UTF)
- `NO_AUTO_CAPTURE`: Disable numbered capturing par theses. (named ones available)
- `NO_AUTO_POSSESS`: Disable auto-possessification.
- `NO_DOTSTAR_ANCHOR`: Disable automatic anchoring for `.*`.
- `NO_START_OPTIMIZE`: Disable match-time start optimizations.
- `NO_UTF_CHECK`: Do not check the pattern for `UTF` valid. (only relevant if `UTF` flag is set)
- `UCP`: Use Unicode properties for `\d`, `\w`, etc.
- `UNGREEDY`: Invert greediness of quantifiers.
- `UTF`: Treat pattern and subjects as UTF strings


### JIT Compile flags

- `JIT_COMPLETE`: compile code for full matching.
- `JIT_PARTIAL_SOFT`: compile code for soft partial matching.
- `JIT_PARTIAL_HARD`: compile code for hard partial matching.


### Match flags

- `ANCHORED`: Match only at the first position.
- `NOTBOL`: Subject string is not the beginning of a line.
- `NOTEOL`: Subject string is not the end of a line.
- `NOTEMPTY`: An empty string is not a valid match.
- `NOTEMPTY_ATSTART`: An empty string at the start of the subject is not a valid match.
- `NO_UTF_CHECK`: Do not check the subject for UTF validity (only relevant if `UTF` flag was set at compile time)
- `PARTIAL_SOFT`: Return `PCRE2_ERROR_PARTIAL` for a partial match if no full matches are found.
- `PARTIAL_HARD`: Return `PCRE2_ERROR_PARTIAL` for a partial match if that is found before a full match.

For details of partial matching, see the `pcre2partial` page.


## Creating a PCRE2 object

### re, err = pcre2.new( pattern [, flag, ...] )

creates a new PCRE2 object.

**Params**

- `pattern:string`: string containing expression to be compiled.
- `flag, ...:number`: [Compile flags](#compile-flags).

**Returns**

- `re:pcre2`: PCRE2 object.
- `err:string`: error message.


## Methods

### ok, err = re:jit_compile( [flag, ...] )

This function requests JIT compilation, which, if the just-in-time compiler is available, further processes a compiled pattern into machine code that executes much faster than the pcre2_match() interpretive matching function. Full details are given in the `pcre2jit` documentation.

**Params**

- `flag, ...:number`: [JIT Compile flags](#jit-compile-flags).

**Returns**

- `ok:boolean`: true on success.
- `err:string`: error message.


### head, tail, err = re:match( sbj [, offset [, flags]] )

matches a compiled regular expression against a given subject string, using a matching algorithm that is similar to Perl's. It returns offsets to captured substrings.

**Params**

- `sbj:string`: the subject string.
- `offset:number`: offset in the subject at which to start matching.
- `flag, ...:number`: [Match flags](#match-flags).

**Returns**

- `head:table`: array of start offsets.
- `tail:table`: array of end offsets.
- `err:string`: error message.


## Example

```lua
local pcre2 = require('pcre2')

local re = assert( pcre2.new('(\\d+)(\\w)') )

assert( re:jit_compile( pcre2.JIT_COMPLETE ) )

local sbj = 'abc081abc134klj567'
local head, tail, err = re:match( sbj )

while head do
    print( 'match' )
    for i = 1, #head do
        print( i, sbj:sub( head[i], tail[i] ) )
    end

    head, tail, err = re:match( sbj, tail[1] )
end

if err then
    error( err )
end

print( 'done' )

--[[
this script will be output the following strings;

match
1	081a
2	081
3	a
match
1	134k
2	134
3	k
match
1	567
2	56
3	7
done
]]
```
