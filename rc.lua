
files   = {}
images  = {}
thumbs  = {}
copies  = {}
frame   = grid.new()
preview = grid.new()

init = function(...)
    for i, path in ipairs({...}) do
        local f = file.new(path)
        local i = image.new(f)
        local t = image.new(f)
        table.insert(files,  f)
        table.insert(images, i)
        table.insert(thumbs, t)
        i:scale(64, 64)
        local c = image.copy(i)
        --table.insert(copies, c)
        i:flip(true)
        --preview:attach(f.image, 1, 1)
        preview:add(i)
        preview:add(c)
    end
    preview:show()
    app:title(false)
    collectgarbage()
end

callme = function()
    print('called me')
end

