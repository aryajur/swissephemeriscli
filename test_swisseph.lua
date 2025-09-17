#!/usr/bin/env luajit

print("Testing Swiss Ephemeris binding...")

local ok, swe = pcall(require, "swisseph")
if not ok then
  print("ERROR: Could not load swisseph module:", swe)
  os.exit(1)
end

print("✓ Swiss Ephemeris module loaded successfully!")

-- Test basic sidereal longitude calculation
print("\nTesting planetary longitude calculation...")

local jd_et = 2451545.0  -- J2000.0
local result, err = pcall(swe.get_sidereal_longitude, "Sun", jd_et, "Lahiri")

if result then
  print("✓ Sun sidereal longitude at J2000.0:", err, "degrees")
else
  print("ERROR during calculation:", err)
  os.exit(1)
end

print("\n✅ Swiss Ephemeris binding test completed successfully!")