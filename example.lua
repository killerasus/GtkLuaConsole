-- Little Lua test

function PrintNumbers ( maxNumber )
    for i=1, maxNumber do
        ConsolePrint( i )
    end
end

function factorial ( value )
    if value <= 1 then
        return 1
    else
        return value * factorial (value - 1)
    end
end


function fibonacci ( term )
    if term == 0 or term == 1 then
        return 1
    else
        return fibonacci( term - 1 ) + fibonacci( term - 2 )
    end
end
