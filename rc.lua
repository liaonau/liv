
--{{{ глобальные переменные
files   = {}
images  = {}
thumbs  = {}
thumbs.size = 64
thumbs.min  = 32
thumbs.max  = 256
frame   = grid.new()
preview = grid.new()

state =
{
    preview = true,
    idx     = 1,
}

default_constraints = {min = 1, max = 2000}

--current  = function() return images[state.idx] end
current  = nil
curthumb = function() return thumbs[state.idx] end
--}}}

--{{{ вспомогательные функции
local function round(num, pow)
    local p = 10^(pow or 0)
    return math.floor(num * p + 0.5) / p
end
--}}}

init = function(...) --{{{
    app.title = 'liv'
    args = {...}
    local square = math.ceil(math.sqrt(#args))
    for n, path in ipairs(args) do
        table.insert(files, path)
        local i = image.new(path)
        local t = image.new(path)
        table.insert(images, i)
        table.insert(thumbs, t)
        local left = (n - 1   ) % square + 1
        local top  = (n - left) / square + 1
        preview:attach(t, left, top)
    end
    thumbs_resize(thumbs.size, thumbs.size)
    preview.homogeneous = {row = false, column = false}
    change_image("last")
    preview:show()
    toggle_preview()
    set_title()
end
--}}}

set_title = function() --{{{
    local spc  = '  |  '
    local apn  = tostring(app)
    local mode = 'mode: ' .. (state.preview and '«preview»' or '«frame»')
    local name = 'file: «'..tostring(current)..'»'..(current.broken and ' (broken)' or '')
    local w, h = size(current)
    local W, H = native_size(current)
    local z    = 100 * (w / W)
    local orig = (w ~= W or h ~= H) and '('..W..'x'..H..')' or ''
    local size = 'size: '..(state.preview and '['..thumbs.size..'px]' or '['..w..'x'..h..']' .. orig .. ' ' ..round(z)..'%')
    local title = apn .. spc .. mode .. spc .. name .. spc .. size
    app.title = title
end
--}}}

--{{{ манипуляции сетками
change_image = function(shift) --{{{
    local max = #images
    if (max == 0) then
        return
    end
    local idx
    if (   shift == "first") then
        idx = 1
    elseif (shift == "last") then
        idx = #images
    else
        idx = state.idx + shift
    end
    idx = (idx > max) and max or idx
    idx = (idx < 1  ) and 1   or idx
    grid.replace(frame, images[idx])
    current = images[idx]
    state.idx = idx
end
--}}}

function grid.replace(g, img) --{{{
    g:clear()
    g:add(img)
end
--}}}

toggle_preview = function() --{{{
    state.preview = not state.preview
    local g = state.preview and preview or frame
    g:show()
end
--}}}
--}}}

--{{{ вспомогательные вычисления размеров
native_size = function(i) --{{{
    local w, h = i.native_width, i.native_height
    if i.swapped then
        w, h = h, w
    end
    return w, h
end
--}}}

size = function(i) --{{{
    if (i == app) then
        return app.width, app.height
    end
    local w, h = i.width, i.height
    return w, h
end
--}}}

inscribe = function(i, W, H) --{{{
    local w, h = native_size(i)
    local a
    if (h > H or w > W) then
        if (h > H) then
            a = H / h
            w, h = w * a, h * a
        end
        if (w > W) then
            a = W / w
            w, h = w * a, h * a
        end
    else
        a = H / h
        w, h = w * a, h * a
        if (w > W) then
            a = W / w
            w, h = w * a, h * a
        end
    end
    return w, h
end
--}}}

fits = function(i, W, H) --{{{
    local w, h = size(i)
    return (w <= W and h <=H)
end
--}}}

native_fits = function(i, W, H) --{{{
    local w, h = native_size(i)
    return (w <= W and h <=H)
end
--}}}

real_min = function(i, min) --{{{
    return math.min(min, math.max(native_size(i)))
end
--}}}

real_max = function(i, max) --{{{
    return math.max(max, math.max(native_size(i)))
end
--}}}

real_constraints = function(i, min, max) --{{{
    return real_min(i, min), real_max(i, max)
end
--}}}
--}}}

--{{{ масштабирование
thumbs_resize = function(s) --{{{
    if (s > thumbs.max) then
        s = thumbs.max
    elseif (s < thumbs.min) then
        s = thumbs.min
    end
    thumbs.size = s
    for i, t in ipairs(thumbs) do
        if (not native_fits(t, s, s)) then
            t:scale(inscribe(t, s, s))
        end
    end
end
--}}}

zoom = function(i, p, min, max) --{{{
    min = min and min or default_constraints.min
    max = max and max or default_constraints.max
    local w, h = size(i)
    local a = w / h
    local s = (100 + p) / 100
    w, h = w * s, h * s
    if ((w / h) ~= a) then
        if (w > h) then h = w / a else w = h * a end
    end
    if (w < min or h < min) then
        if (w <= h) then w, h = min, min / a else w, h = min * a, min end
    elseif (w > max or h > max) then
        if (w >= h) then w, h = max, max / a else w, h = max * a, max end
    end
    i:scale(w, h)
end
--}}}

resizer = --{{{
{
    if_larger = function(i, w, h)
        if (not native_fits(i, w, h)) then
            i:scale(w, h)
        end
    end,
    to_native = function(i) i:scale(native_size(i)) end,
    to_window = function(i) i:scale(size(app)) end,
    to_window_if_larger = function(i)
        if (not native_fits(i, size(app))) then
            resizer.to_window(i)
        end
    end,
    in_window = function(i) i:scale(inscribe(i, size(app))) end,
    in_window_if_larger = function(i)
        if (not native_fits(i, size(app))) then
            resizer.in_window(i)
        end
    end,
    zoom = zoom,
}
--}}}
--}}}

--{{{ скроллинг
function scroll_frame(horizontal, percent) --{{{
    local w, h = size(current)
    if (horizontal) then
        app.hscroll = app.hscroll + percent / 100 * w
    else
        app.vscroll = app.vscroll + percent / 100 * h
    end
end
--}}}

function scroll_preview(horizontal, step) --{{{
    local space = preview.spacing
    if (horizontal) then
        app.hscroll = app.hscroll + step*(thumbs.size + space.row)
    else
        app.vscroll = app.vscroll + step*(thumbs.size + space.column)
    end
end
--}}}
--}}}

--{{{ горячие клавиши
hotkeys = --{{{
{
    any = --{{{
    {
        {{         }, "i", function() print(current) end},

        {{         }, "q", function() app:quit()       end},
        {{         }, "t", function() toggle_preview() end},
    },
    --}}}
    frame = --{{{
    {
        {{         }, "c", function() current:composite() end},

        {{         }, "Left",  function() scroll_frame(true,  -10) end},
        {{         }, "Right", function() scroll_frame(true,   10) end},
        {{         }, "Down",  function() scroll_frame(false,  10) end},
        {{         }, "Up",    function() scroll_frame(false, -10) end},

        {{         }, "j", function() change_image( 1)      end},
        {{         }, "k", function() change_image(-1)      end},
        {{         }, "g", function() change_image("first") end},
        {{"Shift"  }, "g", function() change_image("last")  end},

        {{         }, "s",     function() resizer.to_native(current)           end},
        {{         }, "w",     function() resizer.to_window_if_larger(current) end},
        {{"Control"}, "w",     function() resizer.to_window(current)           end},
        {{"Shift"  }, "a",     function() resizer.in_window(current)           end},
        {{"Control"}, "a",     function() resizer.in_window_if_larger(current) end},
        {{         }, "a", function()
            current:reset()
            resizer.in_window_if_larger(current)
        end},
        {{         }, "minus", function() resizer.zoom(current, -20, real_constraints(current, 64, 1000)) end},
        {{         }, "equal", function() resizer.zoom(current,  20, real_constraints(current, 64, 1000)) end},
        {{         }, "r",            function() current:reset()       end},
        {{         }, "bracketleft",  function() current:rotate(false) end},
        {{         }, "bracketright", function() current:rotate(true)  end},
        {{         }, "f",            function() current:flip(true)    end},
        {{"Control"}, "f",            function() current:flip(false)   end},
    },
    --}}}
    preview = --{{{
    {
        {{         }, "Left",  function() scroll_preview(true,  -1) end},
        {{         }, "Right", function() scroll_preview(true,   1) end},
        {{         }, "Down",  function() scroll_preview(false,  1) end},
        {{         }, "Up",    function() scroll_preview(false, -1) end},

        {{         }, "minus", function() thumbs_resize(thumbs.size - 16) end},
        {{         }, "equal", function() thumbs_resize(thumbs.size + 16) end},
    },
    --}}}
}
--}}}

compair_mods = function(m, h) --{{{
    if (#m ~= #h) then
        return false
    end
    for _, x in ipairs(m) do
        local was = false
        for _, y in ipairs(h) do
            if (x == y) then
                was = true
                break
            end
        end
        if (was == false) then
            return false
        end
    end
    return true
end
--}}}
--}}}

ww, hh = 0, 0
callbacks = --{{{
{

    resize = function(w, h)
        if (ww ~= w or hh ~= h) then
            resizer.in_window_if_larger(current)
            ww, hh = w, h
        end
    end,

    keypress = function(mods, name, value)
        --print(name)
        local specific = state.preview and "preview" or "frame"
        for _, tab in pairs({"any", specific}) do
            for _, hotkey in ipairs(hotkeys[tab]) do
                if (name == hotkey[2]) then
                    if (compair_mods(hotkey[1], mods)) then
                        hotkey[3]()
                        set_title()
                    end
                end
            end
        end
    end,
}
--}}}

