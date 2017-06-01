init = function(path)
    app.status  = false
    app.display = frame.new()
    i           = image.new(path, true)
    app.display.image = i
end
ss = 100
scale = function(s)
    ss = math.min(300, math.max(10, ss + s));
    i:scale((ss/100)*i.native_width, (ss/100)*i.native_height)
end
in_window = function()
    local w,  h  = app:content_size()
    local iw, ih = i.width, i.height
    local nw = (iw > ih) and w       or iw/ih*h
    local nh = (iw > ih) and ih/iw*w or h
    ss = nw/i.native_width*100
    i:scale(nw, nh)
end
keys = {
    q     = function() app.quit()      end,
    d     = function() i:rotate(true ) end,
    a     = function() i:rotate(false) end,
    minus = function() scale(-10)      end,
    equal = function() scale( 10)      end,
    f     = function() i:flip(true )   end,
    v     = function() i:flip(false)   end,
    r     = function() i:reset()       end,
    w     = function() in_window()     end,
}
callbacks = { keypress = function(_, name) keys[name]() end }
