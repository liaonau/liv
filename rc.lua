--{{{ глобальные переменные
files   = {}
images  = {}
thumbs  = {}
marks   = {}
thumbs.min  = 32
thumbs.max  = 256
thumbs.size = thumbs.min
frame   = grid.new('frame')
preview = grid.new('preview')

state =
{
    preview = true,
    idx     = 1,
    update  = true,
}

default_constraints = {min = 1, max = 2000}

current  = nil
--}}}
--{{{{{{ вспомогательные функции
local function round(num, pow)--{{{
    local p = 10^(pow or 0)
    return math.floor(num * p + 0.5) / p
end
--}}}
set_title = function() --{{{
    local function get_state_name(i)--{{{
        local state = i.state
        if     (state == 0) then
            return '0: normal'
        elseif (state == 1) then
            return '1: 90°C'
        elseif (state == 2) then
            return '2: 180°C'
        elseif (state == 3) then
            return '3: 270°C'
        elseif (state == 4) then
            return '4: flipped horizontal'
        elseif (state == 5) then
            return '5: 90°C and flipped horizontal'
        elseif (state == 6) then
            return '6: flipped verticall'
        elseif (state == 7) then
            return '7: 270°C and flipped horizontal'
        end
    end
    --}}}
    local spc   = '  |  '
    local apn   = tostring(app)
    local mode  = 'mode: ' .. (state.preview and '«preview» ['..thumbs.size..'px]' or '«frame»')
    local idx   = '№:'..state.idx
    local name  = 'file: «'..tostring(current)..'»'..(current.broken and ' (broken)' or '')
    local w, h  = dims.size(current)
    local W, H  = dims.native_size(current)
    local orig  = current.broken and '' or ' ('..W..'x'..H..')'
    local mkd   = ((not state.preview and marks[state.idx]) and ' <marked>' or '');
    local z     = 100 * (w / W)
    local size  = (state.preview and '' or spc .. 'size: ['..w..'x'..h..'] '..round(z)..'%')
    local state = (state.preview and '' or spc .. get_state_name(current))
    local pfx   = steward.key_prefix..' '
    local title = apn .. spc .. pfx .. mode .. spc .. idx .. spc .. name .. mkd .. orig .. state .. size
    app.title = title
end
--}}}
thumbs.calc_pos_by_idx = function(idx)--{{{
    local W, H = app.width, app.height
    local spacing = preview.spacing
    local alloc = thumbs[idx].allocation
    local w, h
    --if (allocated) then
        --w = alloc.width  + spacing.row
        --h = alloc.height + spacing.column
    --else
        --w = alloc.natural_width  + spacing.row
        --h = alloc.natural_height + spacing.column
    --end
    w = thumbs.size + spacing.row + 10
    h = thumbs.size + spacing.column + 10
    local constrain = math.max(1, math.floor(W / w))
    local left = ((idx - 1   ) % constrain) + 1
    local top  = ((idx - left) / constrain) + 1
    --if (idx < 30) then
        --print(idx, 'wind', W, H, 'calc', w, h, 'alloc', alloc.width, alloc.height, 'min', alloc.minimum_width, alloc.minimum_height, 'nat', alloc.natural_width, alloc.natural_height)
    --end
    return left, top
end
--}}}
thumbs.pos_by_idx = function(idx)--{{{
    local constrain = preview.size.left
    if (constrain < 1) then
        constrain = 1
    end
    local left = (idx - 1   ) % constrain + 1
    local top  = (idx - left) / constrain + 1
    return left, top
end
--}}}
thumbs.idx_by_pos = function(left, top)--{{{
    return (top - 1) * preview.size.left + left
end
--}}}
--}}}}}}
 --{{{ init
init = function(...)
    app.title = 'liv'
    local f = io.open('liv.css')
    if (f) then
        local style = f:read('*a')
        f:close()
        app:style(style)
    end
    args = {...}
    for n, path in ipairs(args) do
        table.insert(files, path)
        local i = image.new(path)
        local t = image.new(path)
        local w, h = dims.native_size(t);
        thumbs.size = math.min(thumbs.max, math.max(thumbs.min, math.max(w, h)))
        table.insert(images, i)
        table.insert(thumbs, t)
        table.insert(marks, false)
        i.name = 'image'
        t.name = 'thumb'
        t.label = n
    end
    thumbs_resize(thumbs.size, thumbs.size)

    navigator.first()
    viewer.show_preview()
    set_title()
end
--}}}
--{{{{{{ навигация
navigator =
{
next  = function() viewer.move( 1)     end,
prev  = function() viewer.move(-1)     end,
first = function() viewer.set("first") end,
last  = function() viewer.set("last")  end,
index = function(idx, strict)--{{{
    local max = #files
    if (not strict) then
        idx = idx < 1   and 1   or idx
        idx = idx > max and max or idx
    else
        if (idx < 1 or idx > max) then
            return
        end
    end
    viewer.set(idx);
end,
--}}}
direction = function(l, t)--{{{
    local idx = state.idx
    local left, top = thumbs.pos_by_idx(idx)
    local i = thumbs.idx_by_pos(left + l, top + t)
    if (not preview:child(left + l, top + t)) then
        return
    end
    viewer.set(i)
end,
--}}}
up    = function() navigator.direction( 0, -1) end,
down  = function() navigator.direction( 0,  1) end,
left  = function() navigator.direction(-1,  0) end,
right = function() navigator.direction( 1,  0) end,
}
--}}}}}}
--{{{{{{ просмотр
viewer =
{
set = function(idx) --{{{
    local max = #images
    if (max == 0) then
        return
    end
    local old = state.idx
    if (idx == "first") then
        idx = 1
    elseif (idx == "last") then
        idx = max
    end
    idx = idx < 1   and 1   or idx
    idx = idx > max and max or idx

    marker.swap(old, idx)

    current = images[idx]
    state.idx = idx
    frame:clear()
    frame:attach(current, 1, 1)
end,
--}}}
move = function(shift)--{{{
    viewer.set(state.idx + shift)
end,
--}}}
toggle_mode = function() --{{{
    state.preview = not state.preview
    local g = state.preview and preview or frame
    if (state.update) then
        viewer.update[g]()
        state.update = false
    end
    g:show()
end,
--}}}
show_preview = function()--{{{
    state.preview = false
    viewer.toggle_mode()
end,
--}}}
show_frame = function() --{{{
    state.preview = true
    viewer.toggle_mode()
end,
--}}}
fill_preview = function()--{{{
    preview:clear()
    if (#thumbs == 0) then return end
    for idx, t in ipairs(thumbs) do
        local W, H = app.width, app.height
        local spacing = preview.spacing
        local alloc = t.allocation
        local w = math.max(1, alloc.natural_width  + spacing.row)
        local h = math.max(1, alloc.natural_height + spacing.column)
        w = math.max(thumbs.size + math.max(spacing.row, spacing.column), math.max(w, h))
        local constrain = math.max(1, math.floor(W / w))
        local square = math.ceil(math.sqrt(#thumbs))
        constrain = math.min(square, constrain)
        local left = ((idx - 1   ) % constrain) + 1
        local top  = ((idx - left) / constrain) + 1
        if (idx == 8) then
            print(idx,
            'wind', W, H,
            'calc', w, h,
            'alloc', alloc.width, alloc.height,
            'min', alloc.minimum_width, alloc.minimum_height,
            'nat', alloc.natural_width, alloc.natural_height
            )
        end
        preview:attach(t, left, top)
    end
end,
--}}}
update = --{{{
{
    [frame]   = function() resizer.in_window_if_larger(current) end,
    [preview] = function() viewer.fill_preview() end,
},
--}}}
}
--}}}}}}
--{{{{{{ метки
marker =
{
swap = function(old, new)--{{{
    if (marks[old]) then
        images[old].name = 'marked_image'
        thumbs[old].name = 'marked_thumb'
    else
        images[old].name = 'image'
        thumbs[old].name = 'thumb'
    end
    if (marks[new]) then
        images[new].name = 'current_marked_image'
        thumbs[new].name = 'current_marked_thumb'
    else
        images[new].name = 'current_image'
        thumbs[new].name = 'current_thumb'
    end
end,
--}}}
toggle = function(idx)--{{{
    local marked = marks[idx]
    if (state.idx ~= idx) then
        images[idx].name = (marked and 'image' or 'marked_image')
        thumbs[idx].name = (marked and 'thumb' or 'marked_thumb')
    else
        images[idx].name = (marked and 'current_image' or 'current_marked_image')
        thumbs[idx].name = (marked and 'current_thumb' or 'current_marked_thumb')
    end
    marks[idx] = not marks[idx]
end,
--}}}
current_toggle = function()--{{{
    marker.toggle(state.idx)
end,
--}}}
set = function(idx)--{{{
    if (marks[idx]) then
        return
    end
    marker.toggle(idx)
end,
--}}}
unset = function(idx)--{{{
    if (not marks[idx]) then
        return
    end
    marker.toggle(idx)
end,
--}}}
reset = function() --{{{
    for idx, _ in ipairs(files) do
        marker.unset(idx)
    end
end,
--}}}
reverse = function()--{{{
    for idx, _ in ipairs(files) do
        marker.toggle(idx)
    end
end,
--}}}
current_only = function()--{{{
    marker.reset()
    marker.set(state.idx)
end,
--}}}
}
--}}}}}}
--{{{{{{ вычисления размеров
dims =
{
native_size = function(i) --{{{
    local w, h = i.native_width, i.native_height
    if i.swapped then
        w, h = h, w
    end
    return w, h
end,
--}}}
size = function(i) --{{{
    if (i == app) then
        return app.width, app.height
    end
    local w, h = i.width, i.height
    return w, h
end,
--}}}
inscribe = function(i, W, H) --{{{
    local w, h = dims.native_size(i)
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
end,
--}}}
fits = function(i, W, H) --{{{
    local w, h = dims.size(i)
    return (w <= W and h <=H)
end,
--}}}
native_fits = function(i, W, H) --{{{
    local w, h = dims.native_size(i)
    return (w <= W and h <=H)
end,
--}}}
real_min = function(i, min) --{{{
    return math.min(min, math.max(dims.native_size(i)))
end,
--}}}
real_max = function(i, max) --{{{
    return math.max(max, math.max(dims.native_size(i)))
end,
--}}}
real_constraints = function(i, min, max) --{{{
    return dims.real_min(i, min), dims.real_max(i, max)
end,
}
--}}}
--}}}}}}
--{{{{{{ масштабирование
thumbs_resize = function(s) --{{{
    if (s > thumbs.max) then
        s = thumbs.max
    elseif (s < thumbs.min) then
        s = thumbs.min
    end
    thumbs.size = s
    for i, t in ipairs(thumbs) do
        if (not dims.native_fits(t, s, s)) then
            t:scale(dims.inscribe(t, s, s))
        end
    end
end
--}}}
zoom = function(i, p, min, max) --{{{
    min = min and min or default_constraints.min
    max = max and max or default_constraints.max
    local w, h = dims.size(i)
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
        if (not dims.native_fits(i, w, h)) then
            i:scale(w, h)
        end
    end,
    to_native = function(i) i:scale(dims.native_size(i)) end,
    to_window = function(i) i:scale(dims.size(app)) end,
    to_window_if_larger = function(i)
        if (not dims.native_fits(i, dims.size(app))) then
            resizer.to_window(i)
        end
    end,
    in_window = function(i) i:scale(dims.inscribe(i, dims.size(app))) end,
    in_window_if_larger = function(i)
        if (not dims.native_fits(i, dims.size(app))) then
            resizer.in_window(i)
        end
    end,
    zoom = zoom,
}
--}}}
--}}}}}}
--{{{{{{ скроллинг
function scroll_frame(horizontal, percent) --{{{
    local w, h = dims.size(current)
    if (horizontal) then
        app.hscroll = app.hscroll + percent / 100 * w
    else
        app.vscroll = app.vscroll + percent / 100 * h
    end
end
--}}}
function scroll_preview(horizontal, step) --{{{
    local space = preview.spacing
    --local border = 10
    --if (horizontal) then
        --app.hscroll = app.hscroll + step*(thumbs.size + space.row + border)
    --else
        --app.vscroll = app.vscroll + step*(thumbs.size + space.column + border)
    --end
    local alloc = thumbs[state.idx].allocation
    if (horizontal) then
        app.hscroll = app.hscroll + step*(alloc.width + space.row)
    else
        app.vscroll = app.vscroll + step*(alloc.height + space.column)
    end
end
--}}}
--}}}}}}
test = function()
    --local alloc = thumbs[state.idx].allocation
    --print('wind', app.width, app.height, 'alloc', alloc.width, alloc.height, 'min', alloc.minimum_width, alloc.minimum_height, 'nat', alloc.natural_width, alloc.natural_height)
    --local i = preview:child(2, 1)
    local i = thumbs[8]
    local sz = preview.size
    print('alloc', i, i.allocation.width, i.allocation.height)
    print('size', sz.left, sz.top)
end
--{{{{{{ горячие клавиши
steward = --{{{
{
key_prefix = '',
prefixed = function(func, mul, ...)--{{{
    return
        function(...)
            local n = tonumber(steward.key_prefix)
            steward.erase_prefix()
            if (n == nil) then n = 1 end
            n = n*mul
            func(n, ...)
        end
end,
--}}}
erase_prefix = function()--{{{
    steward.key_prefix = ''
end,
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
end,
--}}}
match = function(hotkey, name, mods)--{{{
    if (name == hotkey[2]) then
        if (steward.compair_mods(hotkey[1], mods)) then
            return true
        end
    end
    return false
end,
--}}}
find = function(mods, name, value) --{{{
    local specific = state.preview and "preview" or "frame"
    for _, tab in pairs({"any", specific}) do
        for _, hotkey in ipairs(hotkeys[tab]) do
            if (steward.match(hotkey, name, mods)) then
                return hotkey
            end
        end
    end
end,
--}}}
call = function(hotkey)--{{{
    if (hotkey) then
        hotkey[3]()
        if (not tonumber(hotkey[2])) then
            steward.erase_prefix()
        end
    else
        steward.erase_prefix()
    end
    set_title()
end,
--}}}
}
--}}}
prefixed = steward.prefixed
hotkeys = --{{{
{
    any = --{{{
    {
        {{         }, "m", function() marker.current_toggle() end},
        {{"Shift"  }, "m", function() marker.reverse()        end},
        {{"Control"}, "m", function() marker.reset()          end},

        {{         }, "g", prefixed(function(n) navigator.index(n, true) end, 1)},
        {{"Shift"  }, "g",         function() navigator.last()  end},
        {{"Shift"  }, "6",         function() navigator.first() end}, -- ^
        {{"Shift"  }, "4",         function() navigator.last()  end}, -- $
        {{         }, "space",     function() navigator.next()  end},
        {{         }, "BackSpace", function() navigator.prev()  end},

        {{"Shift"  }, "Q",      function() app:quit()       end},
        {{         }, "q",      function()
            for idx, path in ipairs(files) do
                if (marks[idx]) then
                    print(path)
                end
            end
            app:quit()
        end},
        {{         }, "t",      function() viewer.toggle_mode() end},
        {{         }, "Return", function() viewer.toggle_mode() end},
    },
    --}}}
    frame = --{{{
    {
        {{         }, "h", function() navigator.prev() end},
        {{         }, "j", function() navigator.next() end},
        {{         }, "k", function() navigator.prev() end},
        {{         }, "l", function() navigator.next() end},

        {{         }, "Left",  prefixed(function(n) scroll_frame(true,  n) end, -10)},
        {{         }, "Right", prefixed(function(n) scroll_frame(true,  n) end,  10)},
        {{         }, "Down",  prefixed(function(n) scroll_frame(false, n) end,  10)},
        {{         }, "Up",    prefixed(function(n) scroll_frame(false, n) end, -10)},

        {{         }, "s",     function() resizer.to_native(current)           end},
        {{         }, "w",     function() resizer.to_window_if_larger(current) end},
        {{"Control"}, "w",     function() resizer.to_window(current)           end},
        {{"Shift"  }, "a",     function() resizer.in_window(current)           end},
        {{"Control"}, "a",     function() resizer.in_window_if_larger(current) end},
        {{         }, "a", function()
            current:reset()
            resizer.in_window_if_larger(current)
        end},
        {{         }, "minus", function() resizer.zoom(current, -20, dims.real_constraints(current, 64, 1000)) end},
        {{         }, "equal", function() resizer.zoom(current,  20, dims.real_constraints(current, 64, 1000)) end},
        {{         }, "r",            function() current:reset()       end},
        {{         }, "bracketleft",  function() current:rotate(false) end},
        {{         }, "bracketright", function() current:rotate(true)  end},
        {{         }, "f",            function() current:flip(true)    end},
        {{"Control"}, "f",            function() current:flip(false)   end},
    },
    --}}}
    preview = --{{{
    {
        {{         }, "i", function() viewer.fill_preview() end},
        {{         }, "u", test },

        {{         }, "h", function() navigator.left()  end},
        {{         }, "j", function() navigator.down()  end},
        {{         }, "k", function() navigator.up()    end},
        {{         }, "l", function() navigator.right() end},

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
--{{{ набор префикса
for n=0, 9 do
    table.insert(hotkeys.any, {{}, tostring(n), function() steward.key_prefix = steward.key_prefix..tostring(n) end})
end
--}}}
--}}}}}}
--{{{{{{ колбеки
local ww, hh
callbacks =
{
resize = function(w, h)--{{{
    if ((ww and hh) and (ww ~= w or hh ~= h)) then
        print('resize', ww, hh, w, h, dims.size(app))
        if (state.preview) then
            viewer.update[preview]()
        else
            viewer.update[frame]()
        end
        state.update = true
    end
    ww, hh = w, h
end,
--}}}
keypress = function(mods, name, value)--{{{
    --print(name)
    steward.call(steward.find(mods, name, value))
end,
--}}}
}
--}}}}}}
