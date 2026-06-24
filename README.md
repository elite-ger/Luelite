# Luelite 0.1

A custom fork of Lua 5.5 designed for retro-style game development.

## About

Luelite extends standard Lua with cooperative multitasking, pattern matching, and a built-in HTTP client. It is the scripting language for the **Retrelite** game engine.

Written in C. Inspired by early 2000s game engine scripting.

## Features

| Feature | Description |
|---------|-------------|
| `wait(t)` | Yield coroutine for `t` seconds |
| `spawn(f)` | Run function in a new coroutine |
| `delay(t, f)` | Call function after `t` seconds |
| `tick()` | Global engine time in seconds |
| `match(v, {})` | Pattern matching with markers |
| `http.get(url)` | HTTP GET requests (no dependencies) |
| `.luelite` | Custom file extension support |

## Quick Example

```lua
-- Multitasking
spawn(function()
    for i = 1, 3 do
        print("Background: " .. i)
        wait(0.5)
    end
end)

-- Pattern matching
local hp = 75
local status = match(hp, {
    [_lt(0)] = "dead",
    [_between(1, 50)] = "wounded",
    [_default()] = "healthy"
})
print("Status: " .. status)
```
```lua
-- HTTP requests
local body = http.get("http://example.com")
print("Got " .. #body .. " bytes")
```
Building
bash
git clone https://github.com/elite-ger/Luelite.git
cd luelite
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
make
Requires Lua 5.5 source in src/lua-5.5.0/.

Installation
Run install.bat as Administrator to:

Copy luelite.exe to %USERPROFILE%\retrelite\

Add to system PATH

Associate .luelite files

Documentation
See docs.html for full documentation with examples.

## License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

Author
Elite Ger
