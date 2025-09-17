#include <lua.h>
#include <lauxlib.h>
#include <math.h>
#include <string.h>

#include "swephexp.h"

static int l_get_sidereal_longitude(lua_State *L) {
  const char *planet = luaL_checkstring(L, 1);
  double jd_et = luaL_checknumber(L, 2);
  const char *ayan = luaL_optstring(L, 3, "Lahiri");

  int ipl = SE_SUN;
  if      (!strcmp(planet, "Sun"))     ipl = SE_SUN;
  else if (!strcmp(planet, "Moon"))    ipl = SE_MOON;
  else if (!strcmp(planet, "Mars"))    ipl = SE_MARS;
  else if (!strcmp(planet, "Mercury")) ipl = SE_MERCURY;
  else if (!strcmp(planet, "Jupiter")) ipl = SE_JUPITER;
  else if (!strcmp(planet, "Venus"))   ipl = SE_VENUS;
  else if (!strcmp(planet, "Saturn"))  ipl = SE_SATURN;
  else if (!strcmp(planet, "Rahu"))    ipl = SE_TRUE_NODE; /* or SE_MEAN_NODE */
  else if (!strcmp(planet, "Ketu"))    ipl = SE_TRUE_NODE; /* compute +180 below */

  int sidm = SE_SIDM_LAHIRI;
  if (!strcmp(ayan, "Lahiri")) sidm = SE_SIDM_LAHIRI;
  /* add more mappings if needed */

  const char *ephe_path = getenv("SE_EPHE_PATH");
  if (ephe_path && *ephe_path) swe_set_ephe_path((char*)ephe_path);

  swe_set_sid_mode(sidm, 0, 0);

  double xx[6]; char serr[256];
  int iflag = SEFLG_SWIEPH | SEFLG_SIDEREAL;
  if (swe_calc(jd_et, ipl, iflag, xx, serr) < 0) {
    return luaL_error(L, "swe_calc error: %s", serr);
  }

  double lon = xx[0];
  if (!strcmp(planet, "Ketu")) {
    lon = fmod(lon + 180.0, 360.0);
    if (lon < 0) lon += 360.0;
  }

  lon = fmod(lon, 360.0);
  if (lon < 0) lon += 360.0;
  lua_pushnumber(L, lon);
  return 1;
}

/* Helpers for rise/set, houses */
static int l_sunrise_time(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_getfield(L, 1, "year"); int year = luaL_checkint(L, -1); lua_pop(L,1);
  lua_getfield(L, 1, "month"); int month = luaL_checkint(L, -1); lua_pop(L,1);
  lua_getfield(L, 1, "day"); int day = luaL_checkint(L, -1); lua_pop(L,1);
  int hour = 0, min = 0, sec = 0;
  double tz = 0.0, lat = 0.0, lon = 0.0, elev = 0.0;
  lua_getfield(L, 1, "hour"); if (!lua_isnil(L,-1)) hour = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "min"); if (!lua_isnil(L,-1)) min = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "sec"); if (!lua_isnil(L,-1)) sec = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "tz"); if (!lua_isnil(L,-1)) tz = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "lat"); if (!lua_isnil(L,-1)) lat = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "lon"); if (!lua_isnil(L,-1)) lon = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "elev"); if (!lua_isnil(L,-1)) elev = lua_tonumber(L,-1); lua_pop(L,1);

  double hour_local = hour + min/60.0 + sec/3600.0;
  double hour_utc = hour_local - tz;
  double jd_ut0 = swe_julday(year, month, day, hour_utc, SE_GREG_CAL);

  double geopos[3]; geopos[0] = lon; geopos[1] = lat; geopos[2] = elev;
  double tret;
  char serr[256];
  int rsmi = SE_CALC_RISE;
  int iflag = SEFLG_SWIEPH;
  int rc = swe_rise_trans(jd_ut0, SE_SUN, NULL, iflag, rsmi, geopos, 0, 0, &tret, serr);
  if (rc < 0) return luaL_error(L, "swe_rise_trans error: %s", serr);

  int y2,m2,d2; double hour_ut2;
  swe_revjul(tret, SE_GREG_CAL, &y2, &m2, &d2, &hour_ut2);
  double hour_local2 = hour_ut2 + tz;
  int hh = (int)floor(hour_local2 + 1e-9);
  int mm = (int)floor((hour_local2 - hh)*60 + 1e-6);
  int ss = (int)floor((((hour_local2 - hh)*60) - mm)*60 + 0.5);
  lua_newtable(L);
  lua_pushinteger(L, y2); lua_setfield(L, -2, "year");
  lua_pushinteger(L, m2); lua_setfield(L, -2, "month");
  lua_pushinteger(L, d2); lua_setfield(L, -2, "day");
  lua_pushinteger(L, hh); lua_setfield(L, -2, "hour");
  lua_pushinteger(L, mm); lua_setfield(L, -2, "min");
  lua_pushinteger(L, ss); lua_setfield(L, -2, "sec");
  lua_pushnumber(L, tz); lua_setfield(L, -2, "tz");
  lua_pushnumber(L, lat); lua_setfield(L, -2, "lat");
  lua_pushnumber(L, lon); lua_setfield(L, -2, "lon");
  lua_pushnumber(L, elev); lua_setfield(L, -2, "elev");
  return 1;
}

static int l_sunset_time(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_getfield(L, 1, "year"); int year = luaL_checkint(L, -1); lua_pop(L,1);
  lua_getfield(L, 1, "month"); int month = luaL_checkint(L, -1); lua_pop(L,1);
  lua_getfield(L, 1, "day"); int day = luaL_checkint(L, -1); lua_pop(L,1);
  int hour = 0, min = 0, sec = 0; double tz = 0.0, lat=0.0, lon=0.0, elev=0.0;
  lua_getfield(L, 1, "hour"); if (!lua_isnil(L,-1)) hour = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "min"); if (!lua_isnil(L,-1)) min = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "sec"); if (!lua_isnil(L,-1)) sec = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "tz"); if (!lua_isnil(L,-1)) tz = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "lat"); if (!lua_isnil(L,-1)) lat = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "lon"); if (!lua_isnil(L,-1)) lon = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "elev"); if (!lua_isnil(L,-1)) elev = lua_tonumber(L,-1); lua_pop(L,1);

  double hour_local = hour + min/60.0 + sec/3600.0;
  double hour_utc = hour_local - tz;
  double jd_ut0 = swe_julday(year, month, day, hour_utc, SE_GREG_CAL);

  double geopos[3]; geopos[0] = lon; geopos[1] = lat; geopos[2] = elev;
  double tret; char serr[256];
  int rsmi = SE_CALC_SET; int iflag = SEFLG_SWIEPH;
  int rc = swe_rise_trans(jd_ut0, SE_SUN, NULL, iflag, rsmi, geopos, 0, 0, &tret, serr);
  if (rc < 0) return luaL_error(L, "swe_rise_trans error: %s", serr);

  int y2,m2,d2; double hour_ut2;
  swe_revjul(tret, SE_GREG_CAL, &y2, &m2, &d2, &hour_ut2);
  double hour_local2 = hour_ut2 + tz;
  int hh = (int)floor(hour_local2 + 1e-9);
  int mm = (int)floor((hour_local2 - hh)*60 + 1e-6);
  int ss = (int)floor((((hour_local2 - hh)*60) - mm)*60 + 0.5);
  lua_newtable(L);
  lua_pushinteger(L, y2); lua_setfield(L, -2, "year");
  lua_pushinteger(L, m2); lua_setfield(L, -2, "month");
  lua_pushinteger(L, d2); lua_setfield(L, -2, "day");
  lua_pushinteger(L, hh); lua_setfield(L, -2, "hour");
  lua_pushinteger(L, mm); lua_setfield(L, -2, "min");
  lua_pushinteger(L, ss); lua_setfield(L, -2, "sec");
  lua_pushnumber(L, tz); lua_setfield(L, -2, "tz");
  lua_pushnumber(L, lat); lua_setfield(L, -2, "lat");
  lua_pushnumber(L, lon); lua_setfield(L, -2, "lon");
  lua_pushnumber(L, elev); lua_setfield(L, -2, "elev");
  return 1;
}

static int l_get_house_cusps(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_getfield(L, 1, "year"); int year = luaL_checkint(L, -1); lua_pop(L,1);
  lua_getfield(L, 1, "month"); int month = luaL_checkint(L, -1); lua_pop(L,1);
  lua_getfield(L, 1, "day"); int day = luaL_checkint(L, -1); lua_pop(L,1);
  int hour = 0, min = 0, sec = 0; double tz = 0.0, lat=0.0, lon=0.0;
  lua_getfield(L, 1, "hour"); if (!lua_isnil(L,-1)) hour = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "min"); if (!lua_isnil(L,-1)) min = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "sec"); if (!lua_isnil(L,-1)) sec = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "tz"); if (!lua_isnil(L,-1)) tz = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "lat"); if (!lua_isnil(L,-1)) lat = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "lon"); if (!lua_isnil(L,-1)) lon = lua_tonumber(L,-1); lua_pop(L,1);

  int hsys = 'P';
  if (lua_istable(L, 2)) {
    lua_getfield(L, 2, "house_system");
    if (!lua_isnil(L,-1)) {
      size_t len; const char *s = lua_tolstring(L,-1,&len);
      if (s && len>0) hsys = (int)s[0];
    }
    lua_pop(L,1);
    const char *ayan = NULL;
    lua_getfield(L, 2, "ayanamsa"); if (!lua_isnil(L,-1)) ayan = lua_tostring(L,-1); lua_pop(L,1);
    int sidm = SE_SIDM_LAHIRI; if (ayan && strcmp(ayan,"Lahiri")!=0) sidm = SE_SIDM_LAHIRI; /* TODO map more */
    swe_set_sid_mode(sidm, 0, 0);
  } else {
    swe_set_sid_mode(SE_SIDM_LAHIRI, 0, 0);
  }

  double hour_local = hour + min/60.0 + sec/3600.0;
  double hour_utc = hour_local - tz;
  double jd_ut = swe_julday(year, month, day, hour_utc, SE_GREG_CAL);

  double cusps[13]; double ascmc[10];
  int iflag = SEFLG_SWIEPH | SEFLG_SIDEREAL;
  int rc = swe_houses_ex(jd_ut, iflag, lat, lon, hsys, cusps, ascmc);
  if (rc == ERR) return luaL_error(L, "swe_houses_ex error");

  lua_newtable(L);
  for (int i=1;i<=12;i++) {
    lua_pushnumber(L, cusps[i]); lua_rawseti(L, -2, i);
  }
  return 1;
}

static int l_get_ascendant_longitude(lua_State *L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  lua_getfield(L, 1, "year"); int year = luaL_checkint(L, -1); lua_pop(L,1);
  lua_getfield(L, 1, "month"); int month = luaL_checkint(L, -1); lua_pop(L,1);
  lua_getfield(L, 1, "day"); int day = luaL_checkint(L, -1); lua_pop(L,1);
  int hour = 0, min = 0, sec = 0; double tz = 0.0, lat=0.0, lon=0.0;
  lua_getfield(L, 1, "hour"); if (!lua_isnil(L,-1)) hour = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "min"); if (!lua_isnil(L,-1)) min = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "sec"); if (!lua_isnil(L,-1)) sec = lua_tointeger(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "tz"); if (!lua_isnil(L,-1)) tz = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "lat"); if (!lua_isnil(L,-1)) lat = lua_tonumber(L,-1); lua_pop(L,1);
  lua_getfield(L, 1, "lon"); if (!lua_isnil(L,-1)) lon = lua_tonumber(L,-1); lua_pop(L,1);

  int hsys = 'P';
  if (lua_istable(L, 2)) {
    lua_getfield(L, 2, "house_system"); if (!lua_isnil(L,-1)) { size_t len; const char *s = lua_tolstring(L,-1,&len); if (s && len>0) hsys = (int)s[0]; } lua_pop(L,1);
    const char *ayan = NULL; lua_getfield(L, 2, "ayanamsa"); if (!lua_isnil(L,-1)) ayan = lua_tostring(L,-1); lua_pop(L,1);
    int sidm = SE_SIDM_LAHIRI; if (ayan && strcmp(ayan,"Lahiri")!=0) sidm = SE_SIDM_LAHIRI; swe_set_sid_mode(sidm, 0, 0);
  } else { swe_set_sid_mode(SE_SIDM_LAHIRI, 0, 0); }

  double hour_local = hour + min/60.0 + sec/3600.0;
  double hour_utc = hour_local - tz;
  double jd_ut = swe_julday(year, month, day, hour_utc, SE_GREG_CAL);
  double cusps[13]; double ascmc[10];
  int iflag = SEFLG_SWIEPH | SEFLG_SIDEREAL;
  int rc = swe_houses_ex(jd_ut, iflag, lat, lon, hsys, cusps, ascmc);
  if (rc == ERR) return luaL_error(L, "swe_houses_ex error");
  lua_pushnumber(L, ascmc[SE_ASC]);
  return 1;
}

static const luaL_Reg lib[] = {
  {"get_sidereal_longitude", l_get_sidereal_longitude},
  {"sunrise_time", l_sunrise_time},
  {"sunset_time", l_sunset_time},
  {"get_house_cusps", l_get_house_cusps},
  {"get_ascendant_longitude", l_get_ascendant_longitude},
  {NULL, NULL}
};

LUALIB_API int luaopen_swisseph(lua_State *L) {
#if LUA_VERSION_NUM >= 502
  luaL_newlib(L, lib);
#else
  luaL_register(L, "swisseph", lib);
#endif
  return 1;
}

/* Backward compatibility if someone requires("lua_swisseph") */
LUALIB_API int luaopen_lua_swisseph(lua_State *L) { return luaopen_swisseph(L); }
