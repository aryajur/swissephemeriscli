# Lua Swiss Ephemeris Binding (C module)

This project builds a Lua 5.1+/LuaJIT native module that wraps a subset of the Swiss Ephemeris C library.

- Module name: `lua_swisseph` (require via `require("lua_swisseph")`)
- Exposed functions (initial):
  - `get_sidereal_longitude(planet_name, jd_et, ayanamsa)` → degrees 0..360
  - `sunrise_time(t)` → TODO
  - `sunset_time(t)` → TODO
  - `get_house_cusps(t, opts)` → TODO
  - `get_ascendant_longitude(t, opts)` → TODO

The module is designed to be simple and portable (C + Makefile), usable with Lua 5.1+ and LuaJIT.

## Official Swiss Ephemeris Source
- Official distribution (Astrodienst): https://www.astro.com/ftp/swisseph/
  - Example tarball: `swe_unix_src_<version>.tar.gz` (e.g., 2.10.xx)
- Documentation: https://www.astro.com/swisseph

Note: There are community mirrors on GitHub, but the authoritative source is the Astrodienst site above.

## Build Requirements
- A C compiler and `make`
- Lua dev headers (Lua 5.1+ or LuaJIT)
  - Provide via `pkg-config` (e.g., `lua5.1`, `luajit`) or set env vars `LUA_INCLUDE`, `LUA_LIBDIR`, `LUA_LIBS`.
- curl or wget (to fetch Swiss Ephemeris sources)

## Quick Start
```
# from this folder
./build.sh            # downloads + builds Swiss Ephemeris and the Lua module
# then in Lua
> local swe = require("lua_swisseph")
> swe.get_sidereal_longitude("Sun", 2451545.0, "Lahiri")
```

## Ephemeris Data Files (JPL Integration)

### High-Precision JPL Data
Swiss Ephemeris achieves maximum accuracy when JPL ephemeris data files are available. This binding automatically detects and uses JPL data files when properly configured.

**JPL Data Files Included** (~2MB total):
- `sepl_18.se1` - Planetary positions (Sun, Moon, planets) for 1800-2399 CE  
- `semo_18.se1` - High-precision Moon positions for 1800-2399 CE
- `seas_18.se1` - Asteroid and lunar node positions for 1800-2399 CE

### Configuration
The binding automatically loads JPL data via the `SE_EPHE_PATH` environment variable:

```bash
# Set ephemeris path to use JPL data
export SE_EPHE_PATH=/path/to/swisseph/ephemeris-data

# Run Lua with high-precision calculations
lua your_script.lua
```

### Usage Examples
```lua
-- High-precision sidereal longitude with JPL data
local swe = require("swisseph")
local longitude = swe.get_sidereal_longitude("Jupiter", 2451545.0, "Lahiri")
print("Jupiter longitude: " .. longitude .. "°")
```

### Data File Coverage
- **Time Range**: 1800 CE - 2399 CE (600 years)
- **Precision**: JPL DE431 ephemeris (~0.001" accuracy for inner planets)
- **Fallback**: Without data files, calculations use built-in Moshier ephemeris (reduced accuracy)

### Technical Implementation
The binding code (`lua_swisseph.c:28-29`) automatically configures the ephemeris path:
```c
const char *ephe_path = getenv("SE_EPHE_PATH");
if (ephe_path && *ephe_path) swe_set_ephe_path((char*)ephe_path);
```

## Notes
- The initial version wires `get_sidereal_longitude`; other functions are placeholders that you can fill once basic integration is confirmed.
- The Lua module uses sidereal mode with the provided ayanamsa (e.g., "Lahiri").

## License
- This binding code is MIT licensed. Swiss Ephemeris has its own license; review terms on the official site.

