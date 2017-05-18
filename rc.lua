--{{{ css: CSS для Gtk3
local css =
[===[
#thumb
{
    border:        0px solid;
    border-color:  #aaaaaa;
}
#marked_thumb
{
    border:        4px solid;
    border-color:  #ffff00;
}
#current_thumb
{
    box-shadow:    0px 0px 5px;
    border:        5px solid;
    border-radius: 10px;
    border-color:  #ff0000;
}
#current_marked_thumb
{
    box-shadow:    0px 0px 5px;
    border:        5px solid;
    border-radius: 10px;
    border-color:  #ff00ff;
}
#window
{
}
]===]
--}}}
--{{{ глобальные переменные
files   = {}
images  = {}
picture = scroll.new()
preview = grid.new()
marks   = {}

images.scroll_step = 10
images.zoom_step   = 20
images.zoom_mul    = 1.2
default_constraints =
{
    min = 32,
    max = 2000,
}

state =
{
    preview = true,
    idx     = 1,
    update  = true,
    labels  = {index = false, path = false},
    size    = {window = nil, content = nil},
}

setmetatable(_G,
{
    __index = function(t, k)
        if (k == 'IMG') then
            return images[state.idx]
        end
    end,
})

local function round(num, pow)--{{{
    local p = 10^(pow or 0)
    return math.floor(num * p + 0.5) / p
end
--}}}
--}}}
--{{{{{{ thumbs: предпросмотр
thumbs =
{
min  = 16,
max  = 256,
size = 128,
step = 16,
max_alloc = function()--{{{
    local maxalloc = {width = 0, height = 0}
    for idx, t in ipairs(thumbs) do
        local alloc = t.allocation
        maxalloc.width  = math.max(maxalloc.width,  alloc.width)
        maxalloc.height = math.max(maxalloc.height, alloc.height)
    end
    return maxalloc
end,
--}}}
max_size = function()--{{{
    local size = 0
    for idx, t in ipairs(thumbs) do
        local w, h = dims.native_size(t);
        size = math.max(size, math.max(w, h))
    end
    return size
end,
--}}}
pos_by_idx = function(idx)--{{{
    local constrain = preview.size.left
    if (constrain < 1) then
        constrain = 1
    end
    local left = (idx - 1   ) % constrain + 1
    local top  = (idx - left) / constrain + 1
    return left, top
end,
--}}}
idx_by_pos = function(left, top)--{{{
    return (top - 1) * preview.size.left + left
end,
--}}}
}
--}}}}}}
--{{{ init
init = function(...)
    app.title = 'liv'
    app:style(css)
    args = {...}
    for n, path in ipairs(args) do
        table.insert(files, path)
        local i = image.new(path)
        table.insert(images, i)
        marks[i] = false
    end
    --IMG:load()
    navigator.first()

    --local size = thumbs.max_size()
    --size = math.min(thumbs.max, math.max(thumbs.min, size))
    --thumbs.size = math.min(thumbs.size, size)
    --resizer.thumbs(thumbs.size)

    --if (#images == 1) then
        viewer.show_picture()
    --else
        --viewer.show_preview()
    --end
    --texter.set_title()
    --texter.set_status()
end
--}}}
--{{{ mkup: pango markup
mkup =
{
bold      = function(text) return '<b>'     .. tostring(text) .. '</b>'     end,
italic    = function(text) return '<i>'     .. tostring(text) .. '</i>'     end,
strike    = function(text) return '<s>'     .. tostring(text) .. '</s>'     end,
underline = function(text) return '<u>'     .. tostring(text) .. '</u>'     end,
monospace = function(text) return '<tt>'    .. tostring(text) .. '</tt>'    end,
big       = function(text) return '<big>'   .. tostring(text) .. '</big>'   end,
small     = function(text) return '<small>' .. tostring(text) .. '</small>' end,

r = function(text) return '<span background="#ff8888">' .. tostring(text) .. '</span>' end,
g = function(text) return '<span background="#88ff88">' .. tostring(text) .. '</span>' end,
b = function(text) return '<span background="#8888ff">' .. tostring(text) .. '</span>' end,
c = function(text) return '<span background="#88ffff">' .. tostring(text) .. '</span>' end,
y = function(text) return '<span background="#ffff88">' .. tostring(text) .. '</span>' end,
m = function(text) return '<span background="#ff88ff">' .. tostring(text) .. '</span>' end,
d = function(text) return '<span background="#000000">' .. tostring(text) .. '</span>' end,
w = function(text) return '<span background="#ffffff">' .. tostring(text) .. '</span>' end,
}
--}}}
--{{{{{{texter: статусы
texter =
{
image_state_name = function(i)--{{{
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
end,
--}}}
set_title = function() --{{{
    local spc   = ' | '
    local apn   = tostring(app)
    local name  = tostring(IMG)..(IMG.broken and ' (broken)' or '')
    local title = apn .. spc .. name
    app.title = title
end,
--}}}
set_status = function() --{{{
    --local cond    = function(cond, s) return cond and s or '' end
    --local spc     = ' '
    --local w, h  = dims.size(IMG)
    --local W, H  = dims.native_size(IMG)
    --local z     = 100 * (w / W)

    --local mode  = mkup.m('mode:')..(state.preview and 'preview ['..thumbs.size..'px]' or 'picture')
    --local idx   = mkup.m('№:')..(IMG.broken and mkup.r(state.idx) or mkup.g(state.idx))..' of '..#images
    --local mkd   = cond(marks[state.idx], mkup.y('mark'));
    --local imgst = cond((not state.preview), mkup.m('state:')..texter.image_state_name(IMG))

    --local pfx   = cond((steward.key_prefix ~= ''), mkup.m('prefix:')..mkup.y(steward.key_prefix))
    --local size  = cond((not IMG.broken), mkup.m('size:')..'['..W..'x'..H..']')
    --local pl, pt = thumbs.pos_by_idx(state.idx)
    --local pos   = cond(state.preview, mkup.m('grid:')..'('..pl..','..pt..')')
    --local scale = cond(not state.preview, mkup.m('scale:')..'['..w..'x'..h..'] '..round(z)..'%')
    --app.status_left  = mode..spc ..idx..spc ..mkd..spc ..size..spc ..imgst
    --app.status_right = pos..spc ..scale .. pfx
end,
--}}}
}
--}}}}}}
--{{{{{{ navigator: навигация
navigator =
{
next  = function() viewer.move( 1)     end,
prev  = function() viewer.move(-1)     end,
first = function() viewer.set("first") end,
last  = function() viewer.set("last")  end,
index = function(idx, strict)--{{{
    local max = #images
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
}
--}}}}}}
--{{{{{{ viewer: просмотр
viewer =
{
set_labels = function(st)--{{{
    if (not st or (st.index == state.labels.index and st.path == state.labels.path)) then
        return
    end
    state.labels.index = st.index
    state.labels.path  = st.path
    for idx, t in ipairs(thumbs) do
        local l = ''
        if (state.labels.index) then
            l = l .. idx .. ' '
        end
        if (state.labels.path) then
            l = l .. string.gsub(t.path, '(.*/)(.*)', '%2')
            --l = l .. t.path
        end
        if (l == '') then
            l = nil
        end
        t.label = l
    end
    --viewer.fill_preview()
end,
--}}}
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

    --marker.swap(old, idx)

    --picture:clear()
    --IMG:unload()
    state.idx = idx
    --if (state.preview) then
        --scroller.preview_adjust_to_current()
    --end
    --IMG:load()
    picture:load(IMG)
    --texter.set_title()
end,
--}}}
move = function(shift)--{{{
    viewer.set(state.idx + shift)
end,
--}}}
toggle_mode = function() --{{{
    state.preview = not state.preview
    local g = state.preview and preview or picture
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
show_picture = function() --{{{
    state.preview = true
    viewer.toggle_mode()
end,
--}}}
fill_preview = function()--{{{
    preview:clear()
    if (#thumbs == 0) then return end
    local maxalloc = 1
    for idx, t in ipairs(thumbs) do
        local alloc = t.allocation
        local w = math.max(1, alloc.natural_width)
        local h = math.max(1, alloc.natural_height)
        local s = math.max(w, h)
        if (maxalloc < s) then
            maxalloc = s
        end
    end
    local spacing = preview.spacing
    maxalloc = maxalloc + math.max(spacing.row, spacing.column)
    local W, H = app.width, app.height
    -- растем вниз
    local constrain = math.max(1, math.floor(W / maxalloc))
    local square = math.ceil(math.sqrt(#thumbs))
    constrain = math.min(square, constrain)
    for idx, t in ipairs(thumbs) do
        local left = ((idx - 1   ) % constrain) + 1
        local top  = ((idx - left) / constrain) + 1
        preview:attach(t, left, top)
    end
end,
--}}}
update = --{{{
{
    [picture]   = function()
    end,
    [preview] = function()
        viewer.fill_preview()
    end,
},
--}}}
}
--}}}}}}
--{{{{{{ marker: метки
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
    for idx, _ in ipairs(images) do
        marker.unset(idx)
    end
end,
--}}}
reverse = function()--{{{
    for idx, _ in ipairs(images) do
        marker.toggle(idx)
    end
end,
--}}}
current_only = function()--{{{
    marker.reset()
    marker.set(state.idx)
end,
--}}}
print = function()--{{{
    for idx, path in ipairs(images) do
        if (marks[idx]) then
            print(path)
        end
    end
end,
--}}}
}
--}}}}}}
--{{{{{{ dims: вычисления размеров
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
    return (w <= W and h <= H)
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
real_constraints = function(i) --{{{
    local min, max = default_constraints.min, default_constraints.max
    return dims.real_min(i, min), dims.real_max(i, max)
end,
}
--}}}
--}}}}}}
--{{{{{{ resizer: масштабирование
resizer =
{
thumbs = function(s) --{{{
    s = math.min(thumbs.max, s)
    s = math.max(thumbs.min, s)
    thumbs.size = s
    for i, t in ipairs(thumbs) do
        if (not dims.native_fits(t, s, s)) then
            t:scale(dims.inscribe(t, s, s))
        else
            t:scale(dims.native_size(t))
        end
    end
    viewer.fill_preview()
end,
--}}}
zoom = function(i, s) --{{{
    local min, max = dims.real_constraints(i)
    local w, h = dims.size(i)
    local W, H = dims.native_size(i)
    local a = w / h
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
end,
--}}}
zoom_add = function(i, p) --{{{
    local w, h = dims.size(i)
    local W, H = dims.native_size(i)
    local dist = math.max(w / W, h / H)
    local s = 1 + p / (100*dist)
    resizer.zoom(i, s)
end,
--}}}
if_larger = function(i, w, h)--{{{
    if (not dims.native_fits(i, w, h)) then
        i:scale(w, h)
    end
end,
--}}}
to_native_aspect = function(i)--{{{
    local W, H = dims.native_size(i)
    local na = W / H
    local w, h = dims.size(i)
    local a = w / h
    if (a > na) then
        w = h * na
    else
        h = w / na
    end
    i:scale(w, h)
end,
--}}}
to_native = function(i)--{{{
    i:scale(dims.native_size(i))
end,
--}}}
to_window = function(i)--{{{
    i:scale(app.width, app.height)
end,
--}}}
to_window_if_larger = function(i)--{{{
    if (not dims.native_fits(i, app.width, app.height)) then
        resizer.to_window(i)
    end
end,
--}}}
in_window = function(i)--{{{
i:scale(dims.inscribe(i, app.width, app.height))
end,
--}}}
in_window_if_larger = function(i)--{{{
    if (not dims.native_fits(i, app.width, app.height)) then
        resizer.in_window(i)
    end
end,
--}}}
}
--}}}}}}
--{{{{{{ scroller: скроллинг
scroller =
{
picture = function(hor, ver) --{{{
    app.hscroll = app.hscroll + app.max_hscroll * hor
    app.vscroll = app.vscroll + app.max_vscroll * ver
end,
--}}}
picture_percent = function(hor, ver, absolute) --{{{
    local ch, ch
    if (absolute) then
        ch, cv = 0, 0
    else
        ch, cv = app.hscroll, app.vscroll
    end
    app.hscroll = ch + app.max_hscroll * hor / 100
    app.vscroll = cv + app.max_vscroll * ver / 100
end,
--}}}
picture_center = function()--{{{
    app.hscroll = 0.5 * (app.max_hscroll - app.width)
    app.vscroll = 0.5 * (app.max_vscroll - app.height)
end,
--}}}
preview = function(left_step, top_step) --{{{
    local spacing = preview.spacing
    local alloc   = thumbs.max_alloc()
    app.hscroll = app.hscroll + left_step*(alloc.width + spacing.row)
    app.vscroll = app.vscroll + top_step*(alloc.height + spacing.column)
end,
--}}}
preview_set = function(left, top) --{{{
    local spacing = preview.spacing
    local alloc   = thumbs.max_alloc()
    app.hscroll = (left - 1)*(alloc.width + spacing.row)
    app.vscroll = (top  - 1)*(alloc.height + spacing.column)
end,
--}}}
preview_adjust_to_current = function()--{{{
    local spacing = preview.spacing
    local alloc = thumbs.max_alloc()
    local cl, ct = thumbs.pos_by_idx(state.idx)
    local w, h = alloc.width + spacing.row, alloc.height + spacing.column
    local L, T = math.ceil(app.hscroll / w) + 1, math.ceil(app.vscroll / h) + 1
    local r, c = math.floor(app.width / w), math.floor(app.height / h)
    local visible = (cl >= L and cl + 1 <= L + r and ct >= T and ct + 1 <= T + c)
    if (not visible) then
        local l = ((cl < L) and (cl) or ((cl + 1>= L + r) and (cl - r + 1) or L))
        local t = ((ct < T) and (ct) or ((ct + 1>= T + c) and (ct - c + 1) or T))
        scroller.preview_set(l, t)
    end
end,
--}}}
preview_center = function(left, top) --{{{
    local spacing = preview.spacing
    local alloc   = thumbs.max_alloc()
    local w,  h   = alloc.width + spacing.row, alloc.height + spacing.column
    local R,  C   = round(app.width/(2*w)), round(app.height/(2*h))
    scroller.preview_set(math.max(1, left - R), math.max(1, top - C))
end,
--}}}
preview_center_on_current = function() --{{{
    scroller.preview_center(thumbs.pos_by_idx(state.idx))
end,
--}}}
}
---}}}}}}
test = function()
    --preview:clear()
    --picture:clear()
    --thumbs = nil
    --images = nil
    --preview = nil
    --picture = nil
    collectgarbage()
end
--{{{{{{ steward: горячие клавиши
steward =
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
    local specific = state.preview and "preview" or "picture"
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
    texter.set_status()
end,
--}}}
}
--}}}}}}
--{{{{{{ hotkeys: горячие клавиши
prefixed = steward.prefixed
hotkeys =
{
    any = --{{{
    {
        {{"Control"}, "i", function() viewer.fill_preview() end},
        {{         }, "i", test},

        {{         }, "m", function() marker.current_toggle() end},
        {{"Shift"  }, "m", function() marker.reverse()        end},
        {{"Control"}, "m", function() marker.reset()          end},
        {{         }, "y", function() marker.current_toggle() end},
        {{"Shift"  }, "y", function() marker.reverse()        end},
        {{"Control"}, "y", function() marker.reset()          end},

        {{"Control"}, "r", function() for idx, i in ipairs(images) do i:reset() end end},
        {{"Control"}, "a", function() for idx, i in ipairs(images) do resizer.in_window_if_larger(i) end end},
        {{"Control"}, "w", function() for idx, i in ipairs(images) do resizer.to_window_if_larger(i) end end},
        {{"Control"}, "s", function() for idx, i in ipairs(images) do resizer.to_native(i) end end},

        {{         }, "b", function() app.status_visible = not app.status_visible end},

        {{         }, "g", prefixed(function(n) navigator.index(n,              true) end,  1)},
        {{"Shift"  }, "g", prefixed(function(n) navigator.index(#images + 1 + n, true) end, -1)},
        {{"Shift"  }, "6",         function() navigator.first() end}, -- ^
        {{"Shift"  }, "4",         function() navigator.last()  end}, -- $
        {{         }, "space",     function() navigator.next()  end},
        {{         }, "BackSpace", function() navigator.prev()  end},

        {{"Shift"  }, "Q",      function() app:quit() end},
        {{         }, "q",      function() marker.print(); app:quit() end},
        {{"Shift"  }, "Return", function() marker.print(); app:quit() end},

        {{         }, "t",      function() viewer.toggle_mode() end},
        {{         }, "Return", function() viewer.toggle_mode() end},
    },
    --}}}
    picture = --{{{
    {
        {{         }, "h", function() navigator.prev() end},
        {{         }, "j", function() navigator.next() end},
        {{         }, "k", function() navigator.prev() end},
        {{         }, "l", function() navigator.next() end},

        {{         }, "Left",  prefixed(function(n) scroller.picture_percent(-n,  0) end, images.scroll_step)},
        {{         }, "Right", prefixed(function(n) scroller.picture_percent( n,  0) end, images.scroll_step)},
        {{         }, "Down",  prefixed(function(n) scroller.picture_percent( 0,  n) end, images.scroll_step)},
        {{         }, "Up",    prefixed(function(n) scroller.picture_percent( 0, -n) end, images.scroll_step)},
        {{"Shift"  }, "c",     function(n) scroller.picture_center() end},
        {{"Shift"  }, "Left",  function(n) scroller.picture(-1,  0) end},
        {{"Shift"  }, "Right", function(n) scroller.picture( 1,  0) end},
        {{"Shift"  }, "Down",  function(n) scroller.picture( 0,  1) end},
        {{"Shift"  }, "Up",    function(n) scroller.picture( 0, -1) end},

        {{         }, "s",     function() resizer.to_native_aspect(IMG)    end},
        {{"Shift"  }, "s",     function() resizer.to_native(IMG)           end},
        {{         }, "w",     function() resizer.to_window_if_larger(IMG) end},
        {{"Shift"  }, "w",     function() resizer.to_window(IMG)           end},
        {{         }, "a",     function() resizer.in_window_if_larger(IMG) end},
        {{"Shift"  }, "a",     function() resizer.in_window(IMG)           end},
        {{         }, "minus", prefixed(function(n) resizer.zoom(IMG, 1/n) end, images.zoom_mul)},
        {{         }, "equal", prefixed(function(n) resizer.zoom(IMG,   n) end, images.zoom_mul)},
        {{"Shift"  }, "minus", prefixed(function(n) resizer.zoom_add(IMG, -n) end, images.zoom_step)},
        {{"Shift"  }, "equal", prefixed(function(n) resizer.zoom_add(IMG,  n) end, images.zoom_step)},

        {{         }, "r",            function() IMG:reset()       end},
        {{         }, "bracketleft",  function() IMG:rotate(false) end},
        {{         }, "bracketright", function() IMG:rotate(true)  end},
        {{         }, "f",            function() IMG:flip(true)    end},
        {{         }, "v",            function() IMG:flip(false)   end},
    },
    --}}}
    preview = --{{{
    {
        {{         }, "h", prefixed(function(n) navigator.direction(n, 0) end, -1)},
        {{         }, "j", prefixed(function(n) navigator.direction(0, n) end,  1)},
        {{         }, "k", prefixed(function(n) navigator.direction(0, n) end, -1)},
        {{         }, "l", prefixed(function(n) navigator.direction(n, 0) end,  1)},

        {{"Shift"  }, "l", function() viewer.set_labels({index = state.labels.index,     path = not state.labels.path}) end},
        {{"Shift"  }, "n", function() viewer.set_labels({index = not state.labels.index, path = state.labels.path})     end},
        {{"Control"}, "l", function() viewer.set_labels({index = true,  path = true})  end},
        {{"Control"}, "n", function() viewer.set_labels({index = false, path = false}) end},

        {{         }, "Left",  prefixed(function(n) scroller.preview(n, 0)  end, -1)},
        {{         }, "Right", prefixed(function(n) scroller.preview(n, 0)  end,  1)},
        {{         }, "Down",  prefixed(function(n) scroller.preview(0, n)  end,  1)},
        {{         }, "Up",    prefixed(function(n) scroller.preview(0, n)  end, -1)},
        {{"Shift"  }, "c",     function() scroller.preview_center_on_current() end},

        {{         }, "minus", prefixed(function(n) resizer.thumbs(thumbs.size - n) end, thumbs.step)},
        {{         }, "equal", prefixed(function(n) resizer.thumbs(thumbs.size + n) end, thumbs.step)},
    },
    --}}}
}
--{{{ набор префикса
for n=0, 9 do
    table.insert(hotkeys.any, {{}, tostring(n), function() steward.key_prefix = steward.key_prefix..tostring(n) end})
end
--}}}
--}}}}}}
--{{{{{{ callbacks: колбеки
callbacks =
{
size = function(caller, x, y, w, h)--{{{
    if (state.size[caller] and (state.size[caller].w ~= w or state.size[caller].h ~= h)) then
    print('resize', caller, x, y, w, h)
        --if (state.preview) then
            --viewer.update[preview]()
        --else
            --viewer.update[picture]()
        --end
        --state.update = true
    end
    state.size[caller] = {w = w, h = h}
end,
--}}}
keypress = function(mods, name, value)--{{{
    --print(name)
    steward.call(steward.find(mods, name, value))
end,
--}}}
}
--}}}}}}
