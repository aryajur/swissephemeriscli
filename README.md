# Lua Swiss Ephemeris Binding (C module)

This project builds a Lua 5.1+/LuaJIT native module that wraps a subset of the Swiss Ephemeris C library.

The module is designed to be simple and portable (C + Makefile), usable with Lua 5.1+ and LuaJIT.

## API Documentation

### Module Loading
```lua
local swe = require("swisseph")
-- or
local swe = require("lua_swisseph")  -- backward compatibility
```

### Functions

#### `get_sidereal_longitude(planet_name, jd_et, ayanamsa)`
Calculate the sidereal longitude of a celestial body.

**Parameters:**
- `planet_name` (string): Name of the planet/body. Supported values:
  - `"Sun"`, `"Moon"`, `"Mars"`, `"Mercury"`, `"Jupiter"`, `"Venus"`, `"Saturn"`
  - `"Rahu"` (North Node), `"Ketu"` (South Node - calculated as Rahu + 180°)
- `jd_et` (number): Julian Day in Ephemeris Time (ET/TT)
- `ayanamsa` (string, optional): Ayanamsa system. Default: `"Lahiri"`

**Returns:**
- (number): Sidereal longitude in degrees (0-360)

**Example:**
```lua
local longitude = swe.get_sidereal_longitude("Jupiter", 2451545.0, "Lahiri")
print(string.format("Jupiter: %.4f°", longitude))
```

#### `sunrise_time(datetime_table)`
Calculate sunrise time for a given location and date.

**Parameters:**
- `datetime_table` (table): Table containing:
  - `year` (number, required): Year
  - `month` (number, required): Month (1-12)
  - `day` (number, required): Day of month
  - `hour` (number, optional): Hour (0-23), default: 0
  - `min` (number, optional): Minute (0-59), default: 0
  - `sec` (number, optional): Second (0-59), default: 0
  - `tz` (number, optional): Timezone offset in hours, default: 0
  - `lat` (number, optional): Latitude in degrees, default: 0
  - `lon` (number, optional): Longitude in degrees, default: 0
  - `elev` (number, optional): Elevation in meters, default: 0

**Returns:**
- (table): Datetime table with sunrise time (same fields as input)

**Example:**
```lua
local dt = {
  year = 2024, month = 3, day = 21,
  lat = 28.6139, lon = 77.2090,  -- New Delhi
  tz = 5.5  -- IST
}
local sunrise = swe.sunrise_time(dt)
print(string.format("Sunrise: %02d:%02d:%02d",
  sunrise.hour, sunrise.min, sunrise.sec))
```

#### `sunset_time(datetime_table)`
Calculate sunset time for a given location and date.

**Parameters:**
- Same as `sunrise_time()`

**Returns:**
- (table): Datetime table with sunset time

**Example:**
```lua
local sunset = swe.sunset_time(dt)
print(string.format("Sunset: %02d:%02d:%02d",
  sunset.hour, sunset.min, sunset.sec))
```

#### `get_house_cusps(datetime_table, options)`
Calculate house cusps for a birth chart.

**Parameters:**
- `datetime_table` (table): Same format as `sunrise_time()`
- `options` (table, optional):
  - `house_system` (string): House system character. Default: `"P"` (Placidus)
    - Common systems: `"P"` (Placidus), `"K"` (Koch), `"E"` (Equal), `"W"` (Whole Sign)
  - `ayanamsa` (string): Ayanamsa system. Default: `"Lahiri"`

**Returns:**
- (table): Array of 12 house cusp longitudes (1-indexed)

**Example:**
```lua
local dt = {
  year = 1990, month = 5, day = 15,
  hour = 14, min = 30,
  lat = 19.0760, lon = 72.8777,  -- Mumbai
  tz = 5.5
}
local cusps = swe.get_house_cusps(dt, {house_system = "P"})
for i = 1, 12 do
  print(string.format("House %d: %.2f°", i, cusps[i]))
end
```

#### `get_ascendant_longitude(datetime_table, options)`
Calculate the Ascendant (rising sign) longitude.

**Parameters:**
- Same as `get_house_cusps()`

**Returns:**
- (number): Ascendant longitude in degrees (0-360)

**Example:**
```lua
local asc = swe.get_ascendant_longitude(dt, {ayanamsa = "Lahiri"})
print(string.format("Ascendant: %.2f°", asc))
```

### Complete Example
```lua
local swe = require("swisseph")

-- Birth data
local birth = {
  year = 1975, month = 4, day = 14,
  hour = 3, min = 15, sec = 0,
  lat = 51.5074, lon = -0.1278,  -- London
  tz = 0  -- UTC
}

-- Calculate Julian Day
local jd = 2442509.635417  -- Pre-calculated or use conversion

-- Get planet positions
local planets = {"Sun", "Moon", "Mars", "Mercury", "Jupiter", "Venus", "Saturn", "Rahu"}
for _, planet in ipairs(planets) do
  local lon = swe.get_sidereal_longitude(planet, jd, "Lahiri")
  print(string.format("%s: %.4f°", planet, lon))
end

-- Get house cusps and ascendant
local cusps = swe.get_house_cusps(birth)
local asc = swe.get_ascendant_longitude(birth)
print(string.format("Ascendant: %.2f°", asc))
```

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
```bash
# Build the module
./build.sh            # downloads + builds Swiss Ephemeris and the Lua module

# Set ephemeris data path (optional, for high precision)
export SE_EPHE_PATH=$(pwd)/ephemeris-data

# Test in Lua
lua test_swisseph.lua
```

Or use interactively:
```lua
local swe = require("swisseph")
print(swe.get_sidereal_longitude("Sun", 2451545.0, "Lahiri"))
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

## Notes on Julian Day Calculation

Swiss Ephemeris uses Julian Day numbers for all astronomical calculations. You can calculate Julian Day using:
- External libraries or online calculators
- Swiss Ephemeris provides `swe_julday()` internally (not exposed in this binding yet)
- Basic formula: JD = day + (153 * m + 2)/5 + 365 * y + y/4 - y/100 + y/400 - 32045
  (where m and y are adjusted month and year)

For convenience, consider using an existing Lua date/time library that provides Julian Day conversion.

## License
- This binding code is MIT licensed. Swiss Ephemeris has its own license; review terms on the official site.

