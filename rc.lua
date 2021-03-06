--[[{{{Copyright © 2017 Roman Leonov <rliaonau@gmail.com>
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
--}}}]]
--{{{ документация

--- Конфигурационный скрипт программы Liv.
--  @module rc
--  @author Roman Leonov (Роман Леонов)
--  @copyright © 2017 Roman Leonov <rliaonau@gmail.com>
--  @license GNU General Public License version 3

--- app — Таблица интерфейс к C API.
-- @section app

--- Строка заголовка окна программы.
-- @field title string

--- Таблица статуса программы.
-- @field status table. Поля right, left — string. visible — boolean.

--- Таблица состояния горизонтального ползунка прокрутки.
-- @field hscroll table. Поля val, min, max.

--- Таблица состояния вертикального ползунка прокрутки.
-- @field vscroll table. Поля val, min, max.

--- Текущий отображаемый виджет.
-- @field display userdata типа frameL или gridL.

--- Текущий размер, отведенный под основной виджет.
-- @function content_size
-- @return Ширина.
-- @return Высота.

--- Текущий размер окна программы.
-- @function window_size
-- @return Ширина.
-- @return Высота.

--- Изменение текущего размера окна программы.
-- @function resize
-- @param width Ширина.
-- @param height Высота.

--- Завершение работы программы.
-- @function quit

--- Применение каскадной таблицы стилей.
-- @function style
-- @param css Строка CSS.

--- Хэш имени файла.
-- @function hash
-- @param path Имя файла.
-- @return Хэш.

--- MD5 сумма файла.
-- @function digest
-- @param path Имя файла.
-- @return Строка с MD5 суммой.

--- Значения путей директорий стандарта XDG.
-- @function xdg
-- @return Таблица с полями home, cache, config и data.

--- Создание директории.
-- @function mk_dir
-- @param path Путь.
-- @param mode Маска доступа.
-- @return Успех — boolean.

--- Проверка является ли путь директорией.
-- @function is_dir
-- @param path Путь.
-- @return Успех — boolean.

--- Проверка является ли файл обычным файлом.
-- @function is_file
-- @param path Путь.
-- @return Успех — boolean.

--- image.
-- @section image

--- Создание нового изображения.
-- @function image.new
-- @param path Путь.
-- @param memorize кэшировать ли в память пиксельный буфер.
-- @return Изображение.

--- Информация об изображении.
-- @function image.info
-- @param path Путь.
-- @return Формат.
-- @return Ширина.
-- @return Высота.

--- Изображение.
-- @type image

--- Нативная ширина изображения.
-- @field native_width number (ro).

--- Нативная высота изображения.
-- @field native_height number (ro).

--- Текущая ширина изображения.
-- @field width number (ro).

--- Текущая высота изображения.
-- @field height number (ro).

--- Текущее состояние изображения.
-- @field state number [0-7] (ro).

--- Повернуто ли изображение.
-- @field swapped boolean (ro).

--- Возможно ли отображение изображения на экране.
-- @field broken boolean (ro).

--- Возможна ли запись изображения на диск в родном формате.
-- @field writeable boolean (ro).

--- Кэширован ли пиксельный буфер изображения в оперативную память.
-- @field memorized boolean (ro).

--- Формат изображения.
-- @field format string (ro).

--- Поворот изображения на угол  π/2.
-- @function rotate
-- @param clockwise Направление по часовой стрелке — boolean.

--- Отражение изображения относительно центральной оси симметрии.
-- @function flip
-- @param horizontal Горизонтальна ли ось — boolean.

--- Установка состояния изображения.
-- @function set_state
-- @param state number [0-7].

--- Масштабирование изображения.
-- @function scale
-- @param width number > 0.
-- @param height number > 0.

--- Сброс состояния и масштаба изображения к значениям по умолчанию.
-- @function reset

--- Запись изображения на диск.
-- @function dump
-- @param path Путь.

--- frame.
-- @section frame

--- Создание нового фрейма.
-- @function frame.new
-- @return Фрейм.

--- Фрейм с изображением и меткой.
-- @type frame

--- Текущее изображение фрейма.
-- @field image Изображение.

--- Текущая метка фрейма.
-- @field label Строка.

--- Текущее CSS имя фрейма.
-- @field name Строка.

--- Очистка фрейма.
-- @function clean

--- Добавление CSS класса фрейма.
-- @function class_add
-- @param class Строка.

--- Удаление CSS класса фрейма.
-- @function class_remove
-- @param class Строка.

--- Получение предпочитаемого размера фрейма.
-- @function preferred_size
-- @return Ширина.
-- @return Высота.

--- Установка размера отображения изображения фрейма.
-- @function size_request
-- @param width Ширина.
-- @param height Высота.

--- grid.
-- @section grid

--- Создание новой сетки.
-- @function grid.new
-- @return Сетка.

--- Сетка фреймов.
-- @type grid

--- Расстояние между фреймами.
-- @field spacing Таблица. Поля row и column — number.

--- Очистка сетки.
-- @function clear

--- Добавление фрейма в определенную позицию в сетки.
-- @function attach
-- @param f Фрейм.
-- @param row Строка.
-- @param column Столбец.

--- Удаление фрейма из определенной позиции сетки.
-- @function detach
-- @param row Строка.
-- @param column Столбец.

--- Сигналы (таблица callbacks).
-- @section callbacks

--- Сигнал изменения размера окна.
-- @function size
-- @within callbacks
-- @param x Горизонтальная координата.
-- @param y Вертикальная координата.
-- @param w Ширина.
-- @param h Высота.

--- Сигнал нажатия клавиши.
-- @function keypress
-- @within callbacks
-- @param mods Таблица модификаторов: Shift, Control, Mod1.
-- @param name Имя клавиши в группе 0.
-- @param value Численное значение клавиши с учетом текущей группы.

--- Сигнал нажатия мыши.
-- @function button
-- @within callbacks
-- @param mods Таблица модификаторов: Shift, Control, Mod1.
-- @param name Имя кнопки.
-- @param x Горизонтальная координата.
-- @param y Вертикальная координата.

--- Сигнал изменения текущего положения ползунка прокрутки.
-- @function scroll_value
-- @within callbacks
-- @param direction Строка: 'h' — горизонталь, 'v' — вертикаль.
-- @param scroll Таблица. Поля val, min, max.

--- Сигнал изменения ползунка прокрутки (не текущего положения).
-- @function scroll
-- @within callbacks
-- @param direction Строка: 'h' — горизонталь, 'v' — вертикаль.
-- @param scroll Таблица. Поля val, min, max.

--}}}
--{{{G, opts     : глобальные переменные
rex = require('rex_pcre')
picture = frame.new()
preview = grid.new()
pics = {}

local startidx = 1
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
    min_size = 32,
    max_size = 256,
    size     = 128,
    step     = 16,
    labels =
    {
        index = true,
        path  = false,
    },
    spacing =
    {
        row    = 10,
        column = 10,
    },
}

setmetatable(_G, {--{{{
    __index = function(t, k)
        if     (k == 'IDX') then return state.idx
        elseif (k == 'IMG') then return pics[state.idx].image
        elseif (k == 'PIC') then return pics[state.idx]
        else return rawget(t, k)
        end
    end,
    __newindex = function(t, k, v)
        if (k == 'IDX') then
            state.idx = v
        else
            rawset(t, k, v)
        end
    end,
})--}}}
math.round = function(num) return math.floor(num + 0.5) end
--}}}
--{{{state       : текущее состояние просмотра
state =
setmetatable(
{
    idx = startidx,

    size =
    {
        w = 0,
        h = 0,
    },

    rows     = 1,
    cols     = 1,
    cols_vis = 1,

    thumbsize   = thumbs.size,
    picture_ixd = 0,

    scroll       = { h = {},  v = {}  },
    delay_scroll = { h = nil, v = nil },
    delay_regrid = false,

    button =
    {
        x = 0,
        y = 0,
    },

    labels = thumbs.labels,
},
{__index = function(t, k) return PIC[k] end})
--}}}
--{{{ css         : CSS для Gtk3
local css =
[===[
#window
{
    background-color: «window_background»;
}
#status
{
    background-color: «label_background»;
}
frame label
{
    background-color: «label_background»;
}

.thumb
{
    border:        «border_width»px «border_type»;
    border-color:  «border_color»;
}
.marked
{
    border:        «border_width»px «marked_border_type»;
    border-color:  «marked_border_color»;
}
#current
{
    box-shadow:    «box_shadow»;
    border:        «border_width»px «current_border_type»;
    border-radius: «border_radius»px;
    border-color:  «current_border_color»;
    transition:    «transition»ms;
}
.marked#current
{
    border-color: mix(«current_border_color», «marked_border_color», 0.5);
}
]===]
local css_replace =
{
    window_background    = '#555555',
    label_background     = '#dddddd',

    border_type          = 'solid',
    border_color         = 'alpha(#dddddd, 0.2)',

    current_border_type  = 'solid',
    current_border_color = '#ff5555',
    box_shadow           = '4px 0px 5px',

    marked_border_type   = 'solid',
    marked_border_color  = '#5555ff',

    border_width        = 5,
    border_radius       = 5,
    transition          = 400,
    transition_type     = 'ease-in-out',
}
css = string.gsub(css, '«([^«»]+)»',
    function (w)
        return tostring(css_replace[w])
    end)
--}}}
--{{{ init        : начальный вызов
init = function(...)
    app.title = app.name
    app.style(css)
    args = {...}

--{{{ читаем файлы
    local max_w, max_h = 1, 1
    for n, path in ipairs(args) do
        if (not app.is_file(path)) then
            texter.warn('«'..path..'» is not a regular file')
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
                path   = path,
                image  = i,
                mark   = false,
                digest = app.digest(path)
            })
        end
    end
--}}}

    if (#pics == 0) then--{{{
        texter.fatal('Nothing to display. Exiting.')
        app.quit();
    end
--}}}
    navigator.index(IDX)
    cacher.init()

--{{{ создаем превью
    preview.spacing = thumbs.spacing
    thumbler.adjust_size(max_w, max_h)
    state.thumbsize = thumbs.size
    for idx, pic in ipairs(pics) do
        pic.thumb = frame.new()
        pic.thumb:class_add('thumb')
        marker.update(idx)
        pic.thumb.label = texter.label_index(idx)
    end
    for idx, pic in ipairs(pics) do
        thumbler.create_thumb(pic)
    end
    thumbler.regrid()
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
--{{{cacher      : кэширование изображений
cacher =
{
access = true,
dir    = nil,
init = function()--{{{
    local cd = app.xdg().cache
    cacher.dir = (cd and cd..'/' or os.getenv('HOME')..'/.')..app.name..'/'
    if (not app.is_dir(cacher.dir)) then
        cacher.access = app.mk_dir(cacher.dir, 5570)
    end
end,
--}}}
calc = function(pic)--{{{
    return app.hash(pic.path), app.digest(pic.path)
end,--}}}
check = function(pic, w, h)--{{{
    if (not cacher.access) then
        return pic.path, true
    end
    local i = pic.thumb.image
    local d = pic.digest
    local p = cacher.dir..d..'.'..w..'.'..h
    local hit = app.is_file(p)
    return p, hit
end,
--}}}
}
--}}}
--{{{texter      : статусы
texter =
{
--{{{ tty ANSI color escapes
ANSI_COLOR_RESET     = string.char(27).."[0m",
ANSI_COLOR_RED       = string.char(27).."[31m",
ANSI_COLOR_GREEN     = string.char(27).."[32m",
ANSI_COLOR_YELLOW    = string.char(27).."[33m",
ANSI_COLOR_BLUE      = string.char(27).."[34m",
ANSI_COLOR_MAGENTA   = string.char(27).."[35m",
ANSI_COLOR_CYAN      = string.char(27).."[36m",
ANSI_COLOR_BG_RED    = string.char(27).."[41m",
ANSI_COLOR_BG_YELLOW = string.char(27).."[43m",
--}}}
info = function(s)--{{{
    io.stderr:write(texter.ANSI_COLOR_MAGENTA..'Lua I: '..app.name..': '..s..texter.ANSI_COLOR_RESET..'\n')
end,--}}}
warn = function(s)--{{{
    io.stderr:write(texter.ANSI_COLOR_BG_YELLOW..'Lua W: '..app.name..': '..s..texter.ANSI_COLOR_RESET..'\n')
end,--}}}
fatal = function(s)--{{{
    io.stderr:write(texter.ANSI_COLOR_BG_RED..'Lua E: '..app.name..': '..s..texter.ANSI_COLOR_RESET..'\n')
end,--}}}
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

label_index = function(idx)--{{{
    return (pics[idx].image.broken and mkup.r or mkup.g)(idx)
end,--}}}
label_path = function(path)--{{{
    local d, n, e = texter.split_path(path)
    return n..(e == '' and '' or '.'..e)
end,--}}}

set_title = function() --{{{
    local spc   = ' | '
    local apn   = tostring(app)
    local name  = state.path
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

    local is_preview = (app.display == preview)
    local mode  = mkup.m('mode:')..(is_preview and 'preview ['..state.thumbsize..'px]' or 'picture')
    local idx   = mkup.m('№:')..(IMG.broken and mkup.r(IDX) or mkup.g(IDX))..' of '..#pics
    local mkd   = cond(state.mark, mkup.y('mark'));

    local pfx   = cond((steward.key_prefix ~= ''), mkup.m('prefix:')..mkup.y(steward.key_prefix))
    local size  = cond((not IMG.broken), mkup.m('size:')..'['..W..'x'..H..']px')
    local pl, pt = thumbler.pos_by_idx(IDX)
    local pos   = cond(is_preview, mkup.m('grid:')..'('..pl..','..pt..')')
    local imgst = cond((not is_preview), mkup.m('state:')..texter.image_state_name(IMG))
    local scale = cond(not is_preview, mkup.m('scale:')..'['..w..'x'..h..']px('..math.round(zw)..'x'..math.round(zh)..')%')
    local fmt   = mkup.m('fmt:')..(IMG.writeable and mkup.g or mkup.r)(IMG.format)
    local mem   = mkup.b(mkup.monospace(IMG.memorized and 'm' or 'd'))
    local left  = mode..spc ..idx..spc ..fmt..spc ..mkd..spc ..mem..spc ..size
    local right = pos..spc ..imgst..spc ..scale ..pfx
    app.status  =
    {
        visible = app.status.visible,
        left    = left,
        right   = right,
    }
end,
--}}}
update = function()--{{{
    texter.set_title()
    texter.set_status()
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
index = function(idx)--{{{
    local max = #pics
    idx = idx < 1   and 1   or idx
    idx = idx > max and max or idx
    viewer.set(idx);
end,
--}}}
direction = function(l, t)--{{{
    local left, top = thumbler.pos_by_idx(IDX)
    if (thumbler.pos_exists(left + l, top + t)) then
        viewer.set(thumbler.idx_by_pos(left + l, top + t))
    end
end,
--}}}
coords = function(x, y)--{{{
    local left, top = thumbler.pos_by_coords(x, y)
    if (thumbler.pos_exists(left, top)) then
        viewer.set(thumbler.idx_by_pos(left, top))
    end
end,
--}}}
}
--}}}
--{{{thumbler    : вычисление параметров сетки
thumbler =
{
create_thumb = function(pic)--{{{
    local ms = thumbs.max_size
    local i  = pic.image
    local w, h = dims.native(i)
    local need_cache = false
    if (not dims.native_fits(i, ms, ms)) then
        w, h = dims.inscribe(i, ms, ms)
        need_cache = true
    end

    local cache_path, hit = nil, true
    local p = pic.path

    if (need_cache) then
        cache_path, hit = cacher.check(pic, ms, ms)
        if (hit) then
            p = cache_path
        end
    end

    pic.thumb.image = image.new(p, true, w, h)
    if (need_cache and not hit) then
        image.new(pic.path, false, w, h):dump(cache_path)
    end
    thumbler.item_resize(pic, state.thumbsize)
end,
--}}}
item_size = function()--{{{
    local psw, psh = 1, 1
    for _, pic in ipairs(pics) do
        local w, h = pic.thumb:preferred_size()
        psw = math.max(psw, w)
        psh = math.max(psh, h)
    end
    local spc = preview.spacing
    return (psw + spc.row), (psh + spc.column)
end,
--}}}
item_resize = function(pic, s)--{{{
    local i = pic.thumb.image
    local w, h = i.width, i.height
    local W, H
    if (not dims.native_fits(i, s, s)) then
        W, H = dims.inscribe(i, s, s)
    else
        W, H = dims.native(i)
    end
    if (w ~= W or h ~= H) then
        i:scale(W, H)
    end
    pic.thumb:size_request(s, s)
end,
--}}}
resize = function(s)--{{{
    s = math.min(thumbs.max_size, s)
    s = math.max(thumbs.min_size, s)
    if (s == state.thumbsize) then
        return
    end
    state.thumbsize = s
    for _, pic in ipairs(pics) do
        thumbler.item_resize(pic, s)
    end
    thumbler.regrid()
end,
--}}}
scale = function(step)--{{{
    thumbler.resize(state.thumbsize + step)
end,
--}}}
regrid = function() --{{{
    if (app.display ~= preview) then
        state.delay_regrid = true
        return
    end
    state.delay_regrid = false
    local cw, ch = app:content_size()
    local iw, ih = thumbler.item_size()
    local square = math.ceil(math.sqrt(#pics))
    local r = math.max(1, math.max(math.min( square, math.floor(cw / iw))))
    local c = math.max(1, math.min(math.ceil(#pics / r), math.floor(ch / ih)))
    state.rows     = r
    state.cols_vis = c
    state.cols     = math.ceil(#pics / r)
    preview:clear()
    for idx, pic in ipairs(pics) do
        local l, t = thumbler.pos_by_idx(idx)
        preview:attach(pic.thumb, l, t)
    end
    scroller.preview_adjust_to_current()
end,
--}}}
adjust_size = function(max_w, max_h)--{{{
    local max_thumb_size = math.max(max_w, max_h)
    thumbs.min_size = math.min(thumbs.min_size, max_thumb_size)
    thumbs.max_size = math.min(thumbs.max_size, max_thumb_size)
    thumbs.size = math.min(thumbs.size, math.min(thumbs.max_size, math.max(thumbs.min_size, max_thumb_size)))
end,
--}}}
pos_by_idx = function(idx)--{{{
    local r = state.rows
    local l = (idx - 1) % r + 1
    local t = (idx - l) / r + 1
    return l, t
end,
--}}}
pos_by_coords = function(coords)--{{{
    local x = coords.x
    local y = coords.y
    local cw, ch = app:content_size()
    local iw, ih = thumbler.item_size()
    local hscr = app.hscroll
    local vscr = app.vscroll

    local xs = (hscr.val + x - (cw / 2)) / iw
    local ys = (vscr.val + y - (ch / 2)) / ih
    local L = math.ceil(math.min((state.rows     / 2), (cw / (2 * iw))) + xs)
    local T = math.ceil(math.min((state.cols_vis / 2), (ch / (2 * ih))) + ys)
    return L, T
end,
--}}}
idx_by_pos = function(l, t)--{{{
    return ((t - 1) * state.rows + l)
end,
--}}}
pos_exists = function(l, t)--{{{
    local idx = thumbler.idx_by_pos(l, t)
    return (l > 0 and t > 0 and l <= state.rows and t <= state.cols and idx > 0 and idx <= #pics)
end,
--}}}
}
--}}}
--{{{viewer      : просмотр
viewer =
{
update_picture = function()--{{{
    if (state.picture_ixd ~= IDX) then
        picture:clear()
        picture.image     = IMG
        state.picture_ixd = IDX
    end
    state.delay_scroll = { h = IMG, v = IMG }
end,
--}}}
update_preview = function()--{{{
    if (state.delay_regrid) then
        thumbler.regrid()
    end
    scroller.preview_adjust_to_current()
end,
--}}}

set = function(idx) --{{{
    local max = #pics
    if (idx == "first") then
        idx = 1
    elseif (idx == "last") then
        idx = max
    end
    idx = math.min(max, math.max(1, idx))

    if (IDX == idx) then
        return
    end

    local oldidx = IDX
    IDX = idx

    marker.update(IDX)
    marker.update(oldidx)

    if (app.display == picture) then
        scroller.store(pics[oldidx].image)
        viewer.update_picture()
    end
    if (app.display == preview) then
        viewer.update_preview()
    end
end,
--}}}
move = function(shift)--{{{
    viewer.set(IDX + shift)
end,
--}}}

toggle_display = function() --{{{
    if (app.display == preview) then
        viewer.show_picture()
    elseif (app.display == picture) then
        viewer.show_preview()
    end
end,
--}}}
show_preview = function()--{{{
    if (app.display == picture) then
        scroller.store(IMG)
    end

    app.display = preview
    viewer.update_preview()
end,
--}}}
show_picture = function() --{{{
    app.display = picture
    viewer.update_picture()
end,
--}}}

set_labels = function(st)--{{{
    if (st.index == state.labels.index and st.path == state.labels.path) then
        return
    end
    state.labels.index = st.index
    state.labels.path  = st.path
    for idx, pic in ipairs(pics) do
        local t = pic.thumb
        local l = nil
        if (state.labels.index or state.labels.path) then
            l = ''
            if (state.labels.index) then
                l = texter.label_index(idx)
            end
            if (state.labels.path) then
                l = l .. ' ' .. texter.label_path(pic.path) .. ' '
            end
        end
        t.label = l
    end
    thumbler.regrid()
end,
--}}}

toggle_status = function()--{{{
    app.status = not app.status.visible
end,
--}}}
}
--}}}
--{{{marker      : метки
marker =
{
update = function(idx)--{{{
    local pic = pics[idx]
    if (pic.mark) then
        pic.thumb:class_add('marked')
    else
        pic.thumb:class_remove('marked')
    end
    local is_current = (IDX == idx)
    pic.thumb.name = (is_current and 'current' or '')
end,
--}}}
toggle = function(idx)--{{{
    local pic = pics[idx]
    pic.mark = not pic.mark
    marker.update(idx)
end,
--}}}
toggle_coords = function(x, y)--{{{
    local l, t = thumbler.pos_by_coords(x, y)
    if (thumbler.pos_exists(l, t)) then
        local idx = thumbler.idx_by_pos(l, t)
        marker.toggle(idx)
    end
end,
--}}}
set = function(idx)--{{{
    if (pics[idx].mark) then
        return
    end
    marker.toggle(idx)
end,
--}}}
unset = function(idx)--{{{
    if (not pics[idx].mark) then
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
print_zero = function()--{{{
    for idx, pic in ipairs(pics) do
        if (pic.mark) then
            io.stdout:write(pic.path..'\0')
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
    i:scale(w, h)
end,
--}}}
zoom_add  = function(i, step) --{{{
    local d = math.max(i.width, i.height)
    local s = (d + step) / d
    transformer.zoom_mul(i, s)
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
    i:scale(w, h)
end,
--}}}
scale_to_native_aspect    = function(i) i:scale(dims.native_aspect(i))             end,
scale_to_native           = function(i) i:scale(dims.native(i))                    end,
scale_to_window           = function(i) i:scale(dims.content(i))                   end,
scale_in_window           = function(i) i:scale(dims.inscribe(i, dims.content(i))) end,
scale_to_window_if_larger = function(i) if (not dims.content_fits(i)) then transformer.scale_to_window(i) end end,
scale_in_window_if_larger = function(i) if (not dims.content_fits(i)) then transformer.scale_in_window(i) end end,

rotate = function(i, clockwise)  i:rotate(clockwise) end,
flip   = function(i, horizontal) i:flip(horizontal)  end,
reset  = function(i)             i:reset()           end,
}
--}}}
--{{{scroller    : скроллинг
scroller =
{
percent = function(hor, ver) --{{{
    local hs = app.hscroll
    local vs = app.vscroll
    app.hscroll = { val = hs.val + hs.max * hor / 100 }
    app.vscroll = { val = vs.val + vs.max * ver / 100 }
end,
--}}}
center = function()--{{{
    local hs = app.hscroll
    local vs = app.vscroll
    local W, H = app.content_size()
    app.hscroll = { val = 0.5 * (hs.max - W - hs.min) }
    app.vscroll = { val = 0.5 * (vs.max - H - vs.min) }
end,
--}}}

store = function(i)--{{{
    local hs = app.hscroll
    local vs = app.vscroll
    state.scroll.h[i] = app.hscroll
    state.scroll.v[i] = app.vscroll
end,
--}}}
restore = function(d, i)--{{{
    local s = state.scroll[d][i]
    if (s) then
        app[d..'scroll'] = s
    end
end,
--}}}

preview_step = function(left_step, top_step) --{{{
    local iw, ih = thumbler.item_size()
    local hs = app.hscroll
    local vs = app.vscroll
    app.hscroll = { val = hs.val + iw * left_step }
    app.vscroll = { val = vs.val + ih * top_step  }
end,
--}}}
preview_set = function(left, top) --{{{
    local iw, ih = thumbler.item_size()
    app.hscroll = { val = (left - 1) * iw }
    app.vscroll = { val = (top  - 1) * ih }
end,
--}}}
preview_adjust_to_current = function()--{{{
    local w,  h  = thumbler.item_size()
    local cl, ct = thumbler.pos_by_idx(IDX)
    local hs  = app.hscroll
    local vs  = app.vscroll
    local W, H = app.content_size()
    local L, T = math.ceil(hs.val / w) + 1, math.ceil(vs.val / h) + 1
    local r, c = math.floor(W / w), math.floor(H / h)
    local visible = (cl >= L and cl + 1 <= L + r and ct >= T and ct + 1 <= T + c)
    if (not visible) then
        local l = ((cl < L) and (cl) or ((cl + 1 >= L + r) and (cl - r + 1) or L))
        local t = ((ct < T) and (ct) or ((ct + 1 >= T + c) and (ct - c + 1) or T))
        scroller.preview_set(l, t)
    end
end,
--}}}
preview_center = function(left, top) --{{{
    local iw, ih = thumbler.item_size()
    local cw, ch = app.content_size()
    local R,  C  = math.round(cw/(2*iw)), math.round(ch/(2*ih))
    scroller.preview_set(math.max(1, left - R), math.max(1, top - C))
end,
--}}}
preview_center_on_current = function() --{{{
    scroller.preview_center(thumbler.pos_by_idx(IDX))
end,
--}}}
}
--}}}
--{{{dumper      : сохранение изображений
dumper =
{
dump = function(idx, infix)--{{{
    local i    = pics[idx].image
    local path = pics[idx].path
    local d, n, e = texter.split_path(path)
    if (infix == '') then
        infix = '.copy.'
    end
    if (not i.writeable) then
        e = 'png'
    end
    if (e ~= '') then
        e = '.'..e
    end
    local newpath = d..n..infix..e
    i:dump(newpath)
end,
--}}}
timestamp = function(idx)--{{{
    dumper.dump(idx, os.date('.%Y.%m.%d-%H.%M.%S'))
end,
--}}}
copy_count = {},
copy = function(idx)--{{{
    local dsc = dumper.copy_count
    if (not dsc[idx]) then
        dsc[idx] = 0
    end
    dsc[idx] = dsc[idx] + 1
    dumper.dump(idx, os.date('.'..dsc[idx]))
end,
--}}}
}
--}}}
--{{{test        : тестовая функция
test = function()
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
    for _, tab in pairs({"any", app.display}) do
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

        {{"Shift"  }, "q",      function() app.quit() end},
        {{         }, "q",      function() marker.print();      app.quit() end},
        {{"Control"}, "q",      function() marker.print_zero(); app.quit() end},

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
        {{         }, "c",     function(n) scroller.center() end},

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
        {{         }, "bracketright", function() transformer.rotate(IMG, true ) end},
        {{"Shift"  }, "bracketleft",  function() transformer.rotate(IMG, false); transformer.scale_in_window_if_larger(IMG) end},
        {{"Shift"  }, "bracketright", function() transformer.rotate(IMG, true ); transformer.scale_in_window_if_larger(IMG) end},
        {{         }, "f",            function() transformer.flip(IMG, true )   end},
        {{         }, "v",            function() transformer.flip(IMG, false)   end},
        {{         }, "r",            function() transformer.reset(IMG)         end},

        {{"Control"}, "d", function() dumper.copy(IDX)      end},
        {{"Control"}, "s", function() dumper.timestamp(IDX) end},
    },
    --}}}
    [preview] = --{{{
    {
        {{         }, "h", prefixed(function(n) navigator.direction(n, 0) end, -1)},
        {{         }, "j", prefixed(function(n) navigator.direction(0, n) end,  1)},
        {{         }, "k", prefixed(function(n) navigator.direction(0, n) end, -1)},
        {{         }, "l", prefixed(function(n) navigator.direction(n, 0) end,  1)},
        {{"Shift"  }, "j", prefixed(function(n) navigator.direction(0, n * state.cols_vis) end,  1)},
        {{"Shift"  }, "k", prefixed(function(n) navigator.direction(0, n * state.cols_vis) end, -1)},

        {{         }, "slash", function() viewer.set_labels({index =     state.labels.index, path = not state.labels.path}) end},
        {{"Shift"  }, "slash", function() viewer.set_labels({index = not state.labels.index, path =     state.labels.path}) end},
        {{"Mod1"   }, "slash", function() viewer.set_labels({index = true,  path = true})  end},
        {{"Control"}, "slash", function() viewer.set_labels({index = false, path = false}) end},

        {{         }, "Left",  prefixed(function(n) scroller.preview_step(n, 0)  end, -1)},
        {{         }, "Right", prefixed(function(n) scroller.preview_step(n, 0)  end,  1)},
        {{         }, "Down",  prefixed(function(n) scroller.preview_step(0, n)  end,  1)},
        {{         }, "Up",    prefixed(function(n) scroller.preview_step(0, n)  end, -1)},
        {{         }, "c",     function() scroller.preview_center_on_current() end},

        {{         }, "minus", prefixed(function(n) thumbler.scale(-n) end, thumbs.step)},
        {{         }, "equal", prefixed(function(n) thumbler.scale( n) end, thumbs.step)},
        {{"Shift"  }, "minus", function() thumbler.resize(thumbs.min_size) end},
        {{"Shift"  }, "equal", function() thumbler.resize(thumbs.max_size) end},
        {{         }, "r",     function() thumbler.resize(thumbs.size)     end},

        {{         }, "button1", function() navigator.coords(state.button)     end},
        {{         }, "button3", function() marker.toggle_coords(state.button) end},

    },
    --}}}
}
--{{{ набор префикса
for n=0, 9 do
    table.insert(hotkeys.any, {{}, tostring(n), function() steward.key_prefix = steward.key_prefix..tostring(n) end})
end
--}}}
--}}}
--{{{callbacks   : коллбэки
callbacks =
{
size = function(x, y, w, h)--{{{
    if (state.size.w ~= w or state.size.h ~= h) then
        thumbler.regrid()

        for idx, pic in ipairs(pics) do
            transformer.scale_in_window_if_larger(pic.image)
        end
    end
    state.size = {w = w, h = h}
end,
--}}}
keypress = function(mods, name, value)--{{{
    steward.call(mods, name, value)
end,
--}}}
button = function(mods, button, x, y)--{{{
    state.button = {x = x, y = y}
    local name  = 'button'..button
    local value = -1 * button
    steward.call(mods, name, value)
end,
--}}}
scroll_value = function(d, s)--{{{
    local item = state.delay_scroll[d]
    if (item) then
        state.delay_scroll[d] = nil
        scroller.restore(d, item)
    end
end,
--}}}
scroll = function(d, s)--{{{
    if (app.display == preview) then
        scroller.preview_center_on_current()
    else
        callbacks.scroll_value(d, s)
    end
end,
--}}}
}
--}}}
