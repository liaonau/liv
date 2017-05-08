conf =
{
    max_thumb_size = 128,
    window =
    {
        width  = 800,
        height = 600,
    },
}

local key_prefix = ''

local call_prefixed = function(func, mul, a, b, c)
    return function(a, b, c)
        local n = tonumber(key_prefix)
        key_prefix = ''
        if (n == nil) then n = 1 end
        n = n*mul
        func(n, a, b, c)
    end
end

local slide_step         = 10;
local preview_slide_step = 10;

keys =
{
    image =
    {
        j = call_prefixed(function(n) set_image(true,  n) end,  1),
        k = call_prefixed(function(n) set_image(true,  n) end, -1),
        g = call_prefixed(function(n) set_image(false, n) end,  1),
        G = call_prefixed(function(n) set_image(false, n) end, -1),
        Right = call_prefixed(function(n) slide(false, n) end,  slide_step),
        Left  = call_prefixed(function(n) slide(false, n) end, -slide_step),
        Up    = call_prefixed(function(n) slide(true,  n) end, -slide_step),
        Down  = call_prefixed(function(n) slide(true,  n) end,  slide_step),
        r = function()
            rotate(true)
        end,
        R = function()
            rotate(false)
        end,
        z = function()
            scale()
        end,
        f = function()
            flip(true)
        end,
        F = function()
            flip(false)
        end,
    },

    preview =
    {
        j = call_prefixed(function(n) set_image(true,  n) end,  1),
        k = call_prefixed(function(n) set_image(true,  n) end, -1),
        g = call_prefixed(function(n) set_image(false, n) end,  1),
        G = call_prefixed(function(n) set_image(false, n) end, -1),
        Right = call_prefixed(function(n) slide(false, n) end,  preview_slide_step),
        Left  = call_prefixed(function(n) slide(false, n) end, -preview_slide_step),
        Up    = call_prefixed(function(n) slide(true,  n) end, -preview_slide_step),
        Down  = call_prefixed(function(n) slide(true,  n) end,  preview_slide_step),
    },

    s = function()
        reset()
    end,
    Return = function(a, b, c)
        toggle_preview()
    end,
    q = function(a, b, c)
        quit()
    end,
    i = function()
        print(is_preview())
    end
}

for n=0,9 do
    local char = tostring(n)
    keys[char] = function(a, b, c)
        key_prefix = key_prefix..char
    end
end
