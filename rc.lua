--{{{G, opts     : глобальные переменные
rex = require('rex_pcre')
picture = scroll.new()
preview = grid.new()
pics = {}
--{{{ опции
images =
{
    memory_lim  = 600*600,
    min_size    = 32,
    max_size    = 2000,
    zoom_step   = 10,
    zoom_mul    = 1.2,
    scroll_step = 10,
}
thumbs =
{
    min_size = 16,
    max_size = 256,
    size     = 128,
    step     = 16,
}
--}}}
setmetatable(_G, {--{{{
    __index = function(t, k)
        if     (k == 'IDX') then return state.idx
        elseif (k == 'IMG') then return pics[state.idx].image
        elseif (k == 'TMB') then return pics[state.idx].thumb
        elseif (k == 'PIC') then return pics[state.idx]
        elseif (k == 'DSP') then return state.display
        else return rawget(t, k)
        end
    end,
    __newindex = function(t, k, v)
        if     (k == 'IDX') then state.idx     = v
        elseif (k == 'DSP') then state.display = v
        else                rawset(t, k, v)
        end
    end,
})--}}}
math.round = function(num) return math.floor(num + 0.5) end
--}}}
--{{{state       : текущее состояние просмотра
state =
setmetatable(
{
    size =
    {
        window  = nil,
        content = nil,
    },

    idx     = 1,
    picixd  = 0,
    display = nil,
},
{__index = function(t, k) return PIC[k] end})
--}}}
--{{{ css         : CSS для Gtk3
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
]===]
--}}}
--{{{thumbler    : вычисление параметров сетки
thumbler =
{
resize = function(s) --{{{
    s = math.min(thumbs.max_size, s)
    s = math.max(thumbs.min_size, s)
    thumbs.size = s
    for i, p in ipairs(pics) do
        if (not dims.native_fits(p.thumb, s, s)) then
            p.thumb:scale(dims.inscribe(p.thumb, s, s))
        else
            p.thumb:scale(dims.native(p.thumb))
        end
    end
end,
--}}}
regrid = function() --{{{
    local items, size = preview:get_size()
    if (items ~= #pics or size ~= thumbs.size) then
        preview:set_size(#pics, thumbs.size)
    end
    for idx = 1, #pics do
        local d, n, e = texter.split_path(pics[idx].path)
        preview:set_label(idx, n..(e == '' and '' or '.'..e))
        --preview:set_label(idx, d..n..(e == '' and '' or '.'..e))
        preview:load(idx, pics[idx].thumb, true)
    end
    local psw, psh = preview:preferred_size()
    local cw,  ch  = app:content_size()
    print(cw, ch)
    local spc = preview.spacing
    local fw, fh = psw + spc.row, psh + spc.column
    local square = math.ceil(math.sqrt(#pics))
    local r = math.max(math.min(square, math.floor(cw / fw)))
    local c = math.min(math.ceil(#pics / r), math.floor(ch / fh))
    print(psw, psh)
    local v = r * c
    for idx = 1, #pics do
    --for idx = 1, math.min(v, #pics) do
        local l = (idx - 1) % r + 1
        local t = (idx - l) / r + 1
        preview:attach(idx, l, t)
    end
end,
--}}}
}
--}}}
--{{{ init        : начальный вызов
init = function(...)
    app.title = app.name
    app:style(css)
    args = {...}
--{{{ читаем файлы
    local max_w, max_h = 1, 1
    for n, path in ipairs(args) do
        if (not app:is_file(path)) then
            io.stderr:write('warning: «'..path..'» is not a regular file\n')
        else
            local memorize = true
            local ok, w, h, format = image.info(path)
            max_w, max_h = math.max(max_w, w), math.max(max_h, h)
            if (not ok or w * h > images.memory_lim) then
                memorize = false
            end
            local i = image.new(path, memorize)
            table.insert(pics,
            {
                path  = path,
                image = i,
                mark  = false,
            })
        end
    end
--}}}
    if (#pics == 0) then--{{{
        io.stderr:write('Nothing to display. Exiting.\n')
        os.exit(0)
    end
--}}}
    preview.spacing =--{{{
    {
        row    = 10,
        column = 10,
    }
--}}}
--{{{ создаем превью
    local max_thumb_size = math.max(max_w, max_h)
    thumbs.min_size = math.min(thumbs.min_size, max_thumb_size)
    thumbs.max_size = math.min(thumbs.max_size, max_thumb_size)
    thumbs.size = math.min(thumbs.size, math.min(thumbs.max_size, math.max(thumbs.min_size, max_thumb_size)))
    for n, pic in ipairs(pics) do
        local tmp = image.new(pic.path, false)
        if (not dims.native_fits(tmp, thumbs.max_size, thumbs.max_size)) then
            tmp:scale(dims.inscribe(tmp, thumbs.max_size, thumbs.max_size))
        end
        pic.thumb = tmp:fork()
        tmp       = nil
    end
    collectgarbage()
    thumbler.resize(thumbs.size)
--}}}
    if (#pics == 1) then--{{{
        viewer.show_picture()
    else
        viewer.show_preview()
    end
--}}}
    texter.update()
end
--}}}
--{{{ mkup        : pango markup
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
--{{{texter      : статусы
texter =
{
success_state =--{{{
{
    val = false,
    msg = '',
    set = false,
},
--}}}
success = function(val, msg)--{{{
    texter.success_state.val = val
    texter.success_state.msg = msg
    texter.success_state.set = true
end,
--}}}
image_state_name = function(i)--{{{
    local state = i.state
    if     (state == 0) then return '0:   0°  '
    elseif (state == 1) then return '1:  90°  '
    elseif (state == 2) then return '2: 180°  '
    elseif (state == 3) then return '3: 270°  '
    elseif (state == 4) then return '4:   0° H'
    elseif (state == 5) then return '5:  90° H'
    elseif (state == 6) then return '6:   0° V'
    elseif (state == 7) then return '7: 270° H'
    end
end,
--}}}
split_path = function(path)--{{{
    local dir, base = rex.match(path, '((?:.*/)?)(.*)$')
    if (not dir ) then dir  = '' end
    if (not base) then base = '' end
    local name, ext = rex.match(base, '(.*?)(?:\\.([^\\.]+))?$')
    if (not name) then name = '' end
    if (not ext)  then ext  = '' end
    return dir, name, ext
end,
--}}}
set_title = function() --{{{
    local spc   = ' | '
    local apn   = tostring(app)
    local name  = state.path..(IMG.broken and ' (broken)' or '')
    local title = apn .. spc .. name
    app.title = title
end,
--}}}
set_status = function() --{{{
    local cond = function(cond, s) return cond and s or '' end
    local spc  = ' '
    local w, h = IMG.width, IMG.height
    local W, H = IMG.native_width, IMG.native_height
    local zw, zh = 100 * (w / W), 100 * (h / H)

    local is_preview = (tostring(DSP) == tostring(preview))
    local mode  = mkup.m('mode:')..(is_preview and 'preview ['..thumbs.size..'px]' or 'picture')
    local idx   = mkup.m('№:')..(IMG.broken and mkup.r(IDX) or mkup.g(IDX))..' of '..#pics
    local mkd   = cond(state.marked, mkup.y('mark'));

    local pfx   = cond((steward.key_prefix ~= ''), mkup.m('prefix:')..mkup.y(steward.key_prefix))
    local size  = cond((not IMG.broken), mkup.m('size:')..'['..W..'x'..H..']px')
    --local pl, pt = thumbs.pos_by_idx(IDX)
    --local pos   = cond(is_preview, mkup.m('grid:')..'('..pl..','..pt..')')
    local pos = ''
    local imgst = cond((not is_preview), mkup.m('state:')..texter.image_state_name(IMG))
    local scale = cond(not is_preview, mkup.m('scale:')..'['..w..'x'..h..']px('..math.round(zw)..'x'..math.round(zh)..')%')
    local scs   = cond(texter.success_state.set, (texter.success_state.val and mkup.g or mkup.r)(texter.success_state.msg))
    local mem   = mkup.y(mkup.monospace(IMG.memorize and 'm' or 'd'))
    app.status_left  = mode..spc ..idx..spc ..mem..spc ..mkd..spc ..size..spc ..scs
    app.status_right = pos..spc ..imgst..spc ..scale ..pfx
end,
--}}}
update = function()--{{{
    texter.set_title()
    texter.set_status()
    texter.success_state.set = false
    texter.success_state.val = false
    texter.success_state.msg = ''
end,
--}}}
}
--}}}
--{{{navigator   : навигация
navigator =
{
next  = function() viewer.move( 1)     end,
prev  = function() viewer.move(-1)     end,
first = function() viewer.set("first") end,
last  = function() viewer.set("last")  end,
index = function(idx, strict)--{{{
    local max = #pics
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
--direction = function(l, t)--{{{
    --local idx = IDX
    --local left, top = thumbs.pos_by_idx(idx)
    --local i = thumbs.idx_by_pos(left + l, top + t)
    --if (not preview:child(left + l, top + t)) then
        --return
    --end
    --viewer.set(i)
--end,
--}}}
}
--}}}
--{{{viewer      : просмотр
viewer =
{
update_picture = function()--{{{
    if (DSP == picture and state.picixd ~= IDX) then
        picture:load(IMG, not IMG.memorize)
        state.picixd = IDX
        --print('теперь № '..IDX)
    end
end,
--}}}
update_preview = function()--{{{
    if (DSP ~= preview) then
        return
    end
    preview:clear()
    if (#pics == 0) then
        return
    end
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

set = function(idx) --{{{
    local max = #pics
    if (idx == "first") then
        idx = 1
    elseif (idx == "last") then
        idx = max
    end
    idx = math.max(idx, 1)
    idx = math.min(idx, max)

    if (IDX == idx) then
        return
    end

    --marker.swap(IDX, idx)

    IDX = idx
    viewer.update_picture()
end,
--}}}
move = function(shift)--{{{
    viewer.set(IDX + shift)
end,
--}}}

toggle_display = function() --{{{
    if (DSP == preview) then
        viewer.show_picture()
    elseif (DSP == picture) then
        viewer.show_preview()
    end
end,
--}}}
show_preview = function()--{{{
    DSP = preview
    app:display(preview)
end,
--}}}
show_picture = function() --{{{
    DSP = picture
    app:display(picture)
    viewer.update_picture()
end,
--}}}

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

toggle_status = function()--{{{
    app.status_visible = not app.status_visible
end,
--}}}
}
--}}}
--{{{marker      : метки
marker =
{
toggle = function(idx)--{{{
    local mark = PIC.mark
    if (IDX ~= idx) then
        TMB.name = (mark and 'thumb' or 'marked_thumb')
    else
        TMB.name = (mark and 'current_thumb' or 'current_marked_thumb')
    end
    PIC.mark = not PIC.mark
end,
--}}}
swap = function(old, new)--{{{
    marker.toggle(old)
    marker.toggle(new)
end,
--}}}
set = function(idx)--{{{
    if (PIC.mark) then
        return
    end
    marker.toggle(idx)
end,
--}}}
unset = function(idx)--{{{
    if (not PIC.mark) then
        return
    end
    marker.toggle(idx)
end,
--}}}
reset = function() --{{{
    for idx, _ in ipairs(pics) do
        marker.unset(idx)
    end
end,
--}}}
reverse = function()--{{{
    for idx, _ in ipairs(pics) do
        marker.toggle(idx)
    end
end,
--}}}
current_only = function()--{{{
    marker.reset()
    marker.set(IDX)
end,
--}}}
print = function()--{{{
    for idx, pic in ipairs(pics) do
        if (pic.mark) then
            print(pic.path)
        end
    end
end,
--}}}
}
--}}}
--{{{dims        : вычисления размеров изображений
dims =
{
native  = function(i) return i.native_width, i.native_height end,
native_aspect = function(i) --{{{
    local W, H = dims.native(i)
    local na = W / H
    local w, h = i.width, i.height
    local a = w / h
    if (a > na) then
        w = h * na
    else
        h = w / na
    end
    return w, h
end,
--}}}

inscribe = function(i, W, H) --{{{
    local w, h = dims.native(i)
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
    local w, h = i.width, i.height
    return (w <= W and h <=H)
end,
--}}}
native_fits = function(i, W, H) --{{{
    local w, h = dims.native(i)
    return (w <= W and h <=H)
end,
--}}}

content_fits = function(i) --{{{
    local w, h = dims.native(i)
    local W, H = app:content_size()
    return (w <= W and h <= H)
end,
--}}}
content = function(i) --{{{
    local w, h = app:content_size()
    if i.swapped then
        w, h = h, w
    end
    return w, h
end,
--}}}

constrains = function(i)--{{{
    local SIZE = math.max(dims.native(i))
    local min, max = math.min(SIZE, images.min_size), math.max(SIZE, images.max_size)
    return min, max
end,
--}}}
}
--}}}
--{{{transformer : масштабирование и повороты изображения
transformer =
{
perform = function(i, func, ...)--{{{
    i[func](i, ...)
    if (i == IMG and DSP == picture) then
        picture:load(i, false)
        i = nil
    end
end,
--}}}

zoom_mul  = function(i, s) --{{{
    local min, max = dims.constrains(i)
    local w, h = i.width, i.height
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
    transformer.perform(i, 'scale', w, h)
end,
--}}}
zoom_add  = function(i, step) --{{{
    transformer.scale_add(i, step, step)
end,
--}}}
scale_add = function(i, step_w, step_h) --{{{
    local w, h = i.width, i.height
    if i.swapped then
        step_w, step_h = step_h, step_w
    end
    local min, max = dims.constrains(i)
    w, h = w + step_w, h + step_h
    w = math.min(max, math.max(min, w))
    h = math.min(max, math.max(min, h))
    transformer.perform(i, 'scale', w, h)
end,
--}}}
scale_to_native_aspect    = function(i) transformer.perform(i, 'scale', dims.native_aspect(i))                end,
scale_to_native           = function(i) transformer.perform(i, 'scale', dims.native(i))                       end,
scale_to_window           = function(i) transformer.perform(i, 'scale', dims.content(i))                      end,
scale_in_window           = function(i) transformer.perform(i, 'scale', dims.inscribe(i, dims.content(i)))    end,
scale_to_window_if_larger = function(i) if (not dims.content_fits(i)) then transformer.scale_to_window(i) end end,
scale_in_window_if_larger = function(i) if (not dims.content_fits(i)) then transformer.scale_in_window(i) end end,

rotate = function(i, clockwise)  transformer.perform(i, 'rotate', clockwise) end,
flip   = function(i, horizontal) transformer.perform(i, 'flip', horizontal)  end,
reset  = function(i)             transformer.perform(i, 'reset')             end,

reload = function(i)--{{{
    local m = i.memorize
    i:memorize()
    if (not m) then
        i:unmemorize()
    end
end,
--}}}
}
--}}}
--{{{scroller    : скроллинг
scroller =
{
percent = function(hor, ver) --{{{
    local hs, vs, min_hs, min_vs, max_hs, max_vs = picture:get_scroll()
    picture:set_scroll(
    {
        h = hs + max_hs * hor / 100,
        v = vs + max_vs * ver / 100,
    })
end,
--}}}
center = function()--{{{
    local hs, vs, min_hs, min_vs, max_hs, max_vs = picture:get_scroll()
    picture:set_scroll(
    {
        h = 0.5 * (max_hs - min_hs),
        v = 0.5 * (max_vs - min_vs),
    })
end,
--}}}
--preview = function(left_step, top_step) --{{{
    --local spacing = preview.spacing
    --local alloc   = thumbs.max_alloc()
    --app.hscroll = app.hscroll + left_step*(alloc.width + spacing.row)
    --app.vscroll = app.vscroll + top_step*(alloc.height + spacing.column)
--end,
----}}}
--preview_set = function(left, top) --{{{
    --local spacing = preview.spacing
    --local alloc   = thumbs.max_alloc()
    --app.hscroll = (left - 1)*(alloc.width + spacing.row)
    --app.vscroll = (top  - 1)*(alloc.height + spacing.column)
--end,
----}}}
--preview_adjust_to_current = function()--{{{
    --local spacing = preview.spacing
    --local alloc = thumbs.max_alloc()
    --local cl, ct = thumbs.pos_by_idx(IDX)
    --local w, h = alloc.width + spacing.row, alloc.height + spacing.column
    --local L, T = math.ceil(app.hscroll / w) + 1, math.ceil(app.vscroll / h) + 1
    --local r, c = math.floor(app.width / w), math.floor(app.height / h)
    --local visible = (cl >= L and cl + 1 <= L + r and ct >= T and ct + 1 <= T + c)
    --if (not visible) then
        --local l = ((cl < L) and (cl) or ((cl + 1>= L + r) and (cl - r + 1) or L))
        --local t = ((ct < T) and (ct) or ((ct + 1>= T + c) and (ct - c + 1) or T))
        --scroller.preview_set(l, t)
    --end
--end,
----}}}
--preview_center = function(left, top) --{{{
    --local spacing = preview.spacing
    --local alloc   = thumbs.max_alloc()
    --local w,  h   = alloc.width + spacing.row, alloc.height + spacing.column
    --local R,  C   = round(app.width/(2*w)), round(app.height/(2*h))
    --scroller.preview_set(math.max(1, left - R), math.max(1, top - C))
--end,
----}}}
--preview_center_on_current = function() --{{{
    --scroller.preview_center(thumbs.pos_by_idx(IDX))
--end,
----}}}
}
---}}}
--{{{dumper      : сохранение изображений
dumper =
{
dump = function(idx, infix)--{{{
    local i    = pics[idx].image
    local path = pics[idx].path
    local d, n, e = texter.split_path(path)
    if (not i.writeable) then
        e = 'png'
    end
    if (e ~= '') then
        e = '.'..e
    end
    local newpath = d..n..infix..e
    local success = i:dump(newpath)
    texter.success(success, 'dump image to «'..newpath..'»')
    return success, newpath
end,
--}}}
copy = function(idx)--{{{
    dumper.dump(idx, os.date('-%Y.%m.%d-%H.%M.%S'))
end,
--}}}
save = function(idx)--{{{
    local success, newpath = dumper.dump(idx, '')
    if (success) then
        pics[idx].image:fix()
        pics[idx].path = newpath
    end
end,
--}}}
}
--}}}
--{{{test        : тестовая функция
test = function()
    thumbler.regrid()
    collectgarbage()
end
--}}}
--{{{steward     : горячие клавиши
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
foreach = function(tab, func, ...)--{{{
    return function(...)
        for k, v in ipairs(tab) do
            func(k, v, ...)
        end
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
    for _, tab in pairs({"any", DSP}) do
        for _, hotkey in ipairs(hotkeys[tab]) do
            if (steward.match(hotkey, name, mods)) then
                return hotkey
            end
        end
    end
end,
--}}}
call = function(mods, name, value)--{{{
    local hotkey = steward.find(mods, name, value)
    if (hotkey) then
        hotkey[3]()
        if (not tonumber(hotkey[2])) then
            steward.erase_prefix()
        end
    else
        steward.erase_prefix()
    end
    texter.update()
end,
--}}}
}
--}}}
--{{{hotkeys     : горячие клавиши
prefixed = steward.prefixed
foreach  = steward.foreach
hotkeys =
{
    any = --{{{
    {
        {{         }, "i", test},
        {{"Control"}, "i", function() print(preview:preferred_size()) end},

        {{         }, "m", function() marker.toggle(IDX) end},
        {{"Shift"  }, "m", function() marker.reverse()   end},
        {{"Control"}, "m", function() marker.reset()     end},
        {{         }, "y", function() marker.toggle(IDX) end},
        {{"Shift"  }, "y", function() marker.reverse()   end},
        {{"Control"}, "y", function() marker.reset()     end},

        {{"Control"}, "r", foreach(pics, function(_, p) transformer.reset(p.image)                     end)},
        {{"Control"}, "a", foreach(pics, function(_, p) transformer.scale_in_window_if_larger(p.image) end)},
        {{"Control"}, "w", foreach(pics, function(_, p) transformer.scale_to_window_if_larger(p.image) end)},
        {{"Control"}, "n", foreach(pics, function(_, p) transformer.scale_to_native(p.image)           end)},

        {{         }, "b", function() viewer.toggle_status() end},

        {{         }, "g", prefixed(function(n) navigator.index(n            ) end,  1)},
        {{"Shift"  }, "g", prefixed(function(n) navigator.index(#pics + 1 + n) end, -1)},
        {{"Shift"  }, "6",         function() navigator.first() end}, -- ^
        {{"Shift"  }, "4",         function() navigator.last()  end}, -- $
        {{         }, "space",     function() navigator.next()  end},
        {{         }, "BackSpace", function() navigator.prev()  end},

        {{"Shift"  }, "Q",      function() app:quit() end},
        {{         }, "q",      function() marker.print(); app:quit() end},
        {{"Shift"  }, "Return", function() marker.print(); app:quit() end},

        {{         }, "t",      function() viewer.toggle_display() end},
        {{         }, "Return", function() viewer.toggle_display() end},
    },
    --}}}
    [picture] = --{{{
    {
        {{         }, "h", function() navigator.prev() end},
        {{         }, "j", function() navigator.next() end},
        {{         }, "k", function() navigator.prev() end},
        {{         }, "l", function() navigator.next() end},

        {{         }, "Left",  prefixed(function(n) scroller.percent(-n,  0) end, images.scroll_step)},
        {{         }, "Right", prefixed(function(n) scroller.percent( n,  0) end, images.scroll_step)},
        {{         }, "Down",  prefixed(function(n) scroller.percent( 0,  n) end, images.scroll_step)},
        {{         }, "Up",    prefixed(function(n) scroller.percent( 0, -n) end, images.scroll_step)},
        {{"Shift"  }, "Left",  function(n) scroller.percent(-100,    0) end},
        {{"Shift"  }, "Right", function(n) scroller.percent( 100,    0) end},
        {{"Shift"  }, "Down",  function(n) scroller.percent(   0,  100) end},
        {{"Shift"  }, "Up",    function(n) scroller.percent(   0, -100) end},
        {{"Shift"  }, "c",     function(n) scroller.center() end},

        {{         }, "n",     function() transformer.scale_to_native(IMG)           end},
        {{"Shift"  }, "n",     function() transformer.scale_to_native_aspect(IMG)    end},
        {{         }, "w",     function() transformer.scale_to_window_if_larger(IMG) end},
        {{"Shift"  }, "w",     function() transformer.scale_to_window(IMG)           end},
        {{         }, "a",     function() transformer.scale_in_window_if_larger(IMG) end},
        {{"Shift"  }, "a",     function() transformer.scale_in_window(IMG)           end},

        {{         }, "minus", prefixed(function(n) transformer.zoom_mul(IMG, 1/n) end, images.zoom_mul)},
        {{         }, "equal", prefixed(function(n) transformer.zoom_mul(IMG,   n) end, images.zoom_mul)},
        {{"Shift"  }, "minus", prefixed(function(n) transformer.zoom_add(IMG,  -n) end, images.zoom_step)},
        {{"Shift"  }, "equal", prefixed(function(n) transformer.zoom_add(IMG,   n) end, images.zoom_step)},

        {{         }, "period",   prefixed(function(n) transformer.scale_add(IMG,  n,  0) end, images.zoom_step)},
        {{         }, "question", prefixed(function(n) transformer.scale_add(IMG, -n,  0) end, images.zoom_step)},
        {{"Shift"  }, "period",   prefixed(function(n) transformer.scale_add(IMG,  0,  n) end, images.zoom_step)},
        {{"Shift"  }, "question", prefixed(function(n) transformer.scale_add(IMG,  0, -n) end, images.zoom_step)},

        {{         }, "bracketleft",  function() transformer.rotate(IMG, false) end},
        {{         }, "bracketright", function() transformer.rotate(IMG, true)  end},
        {{"Shift"  }, "bracketleft",  function() transformer.rotate(IMG, false); transformer.scale_in_window_if_larger(IMG) end},
        {{"Shift"  }, "bracketright", function() transformer.rotate(IMG, true);  transformer.scale_in_window_if_larger(IMG) end},
        {{         }, "f",            function() transformer.flip(IMG, true)    end},
        {{         }, "v",            function() transformer.flip(IMG, false)   end},
        {{         }, "r",            function() transformer.reset(IMG)         end},

        {{"Control"}, "l",            function() transformer.reload(IMG)        end},

        {{         "Control"}, "s", function() dumper.copy(IDX) end},
        {{"Shift", "Control"}, "s", function() dumper.save(IDX) end},
    },
    --}}}
    [preview] = --{{{
    {
        --{{         }, "h", prefixed(function(n) navigator.direction(n, 0) end, -1)},
        --{{         }, "j", prefixed(function(n) navigator.direction(0, n) end,  1)},
        --{{         }, "k", prefixed(function(n) navigator.direction(0, n) end, -1)},
        --{{         }, "l", prefixed(function(n) navigator.direction(n, 0) end,  1)},

        --{{"Shift"  }, "l", function() viewer.set_labels({index = state.labels.index,     path = not state.labels.path}) end},
        --{{"Shift"  }, "n", function() viewer.set_labels({index = not state.labels.index, path = state.labels.path    }) end},
        --{{"Control"}, "l", function() viewer.set_labels({index = true,  path = true})  end},
        --{{"Control"}, "n", function() viewer.set_labels({index = false, path = false}) end},

        --{{         }, "Left",  prefixed(function(n) scroller.preview(n, 0)  end, -1)},
        --{{         }, "Right", prefixed(function(n) scroller.preview(n, 0)  end,  1)},
        --{{         }, "Down",  prefixed(function(n) scroller.preview(0, n)  end,  1)},
        --{{         }, "Up",    prefixed(function(n) scroller.preview(0, n)  end, -1)},
        --{{"Shift"  }, "c",     function() scroller.preview_center_on_current() end},

        --{{         }, "minus", prefixed(function(n) thumbs.scale(thumbs.size - n) end, thumbs.step)},
        --{{         }, "equal", prefixed(function(n) thumbs.scale(thumbs.size + n) end, thumbs.step)},
    },
    --}}}
}
--{{{ набор префикса
for n=0, 9 do
    table.insert(hotkeys.any, {{}, tostring(n), function() steward.key_prefix = steward.key_prefix..tostring(n) end})
end
--}}}
--}}}
--{{{callbacks   : колбеки
callbacks =
{
size = function(caller, x, y, w, h)--{{{
    if (state.size[caller] and (state.size[caller].w ~= w or state.size[caller].h ~= h)) then
    --print('resize', caller, x, y, w, h)
    end
    state.size[caller] = {w = w, h = h}
end,
--}}}
keypress = function(mods, name, value)--{{{
    steward.call(mods, name, value)
end,
--}}}
}
--}}}
