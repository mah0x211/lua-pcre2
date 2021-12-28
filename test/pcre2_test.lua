local assert = assert
local pcre2 = require('pcre2')
local dump = require('dump')

local function test_jit_compile()
    local re = assert(pcre2.new('(\\d+)(\\w)'))
    assert(re:jit_compile(pcre2.JIT_COMPLETE))
end

local function test_match()
    local sbj = 'abc081abc134klj567'
    local re = assert(pcre2.new('(\\d+)(\\w)'))
    local head, tail, err = re:match(sbj)
    local matches = {}

    assert(not err, err)
    while head do
        local list = {}
        for i = 1, #head do
            list[i] = sbj:sub(head[i], tail[i])
        end
        matches[#matches + 1] = list

        head, tail, err = re:match(sbj, tail[1])
        assert(not err, err)
    end

    assert(dump(matches) == dump({
        {
            '081a', -- (\\d+)(\\w)
            '081', -- (\\d+)
            'a', -- (\\w)
        },
        {
            '134k',
            '134',
            'k',
        },
        {
            '567',
            '56',
            '7',
        },
    }))
end

test_jit_compile()
test_match()

