-- Start
-- Script: script.lua
print("I am using Lua from within C")

for i = 1, 10 do
    print(i)
end

table = {
    a = 5,
    b = 6
}

print(table["a"])
print(table.b)

Screen.testme(5)

function _screen_draw()

end
-- End