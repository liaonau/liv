keys = {
    a = function(a, b, c)
        print(a, b, c)
    end,
    j = function(a, b, c)
        print(a, b, c)
        next_image()
    end,
    k = function(a, b, c)
        print(a, b, c)
        prev_image()
    end,
    space = function(a, b, c)
        print(a, b, c)
    end,
    q = function(a, b, c)
        print(a, b, c)
        quit()
    end,
    r = function(a, b, c)
        print(a, b, c)
        rotate(true)
    end,
    R = function(a, b, c)
        print(a, b, c)
        rotate(false)
    end,
}
