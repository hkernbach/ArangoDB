////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Heiko Kernbach
////////////////////////////////////////////////////////////////////////////////

#include "GeoParser.h"
#include "Basics/VelocyPackHelper.h"
#include "Logger/Logger.h"

#include <velocypack/Iterator.h>
#include <velocypack/velocypack-aliases.h>

#include <geometry/s2.h>
#include <geometry/s2loop.h>
#include <geometry/s2polygon.h>
#include <geometry/strings/split.h>
#include <geometry/strings/strutil.h>

#include <vector>
using std::vector;

#include <string>
using std::string;

using namespace arangodb::basics;
using namespace arangodb::aql;

// This field must be present, and...
static const string GEOJSON_TYPE = "type";
// Have one of these values:
static const string GEOJSON_TYPE_POINT = "Point";
static const string GEOJSON_TYPE_LINESTRING = "LineString";
static const string GEOJSON_TYPE_POLYGON = "Polygon";
static const string GEOJSON_TYPE_MULTI_POINT = "MultiPoint";
static const string GEOJSON_TYPE_MULTI_LINESTRING = "MultiLineString";
static const string GEOJSON_TYPE_MULTI_POLYGON = "MultiPolygon";
static const string GEOJSON_TYPE_GEOMETRY_COLLECTION = "GeometryCollection";
// This field must also be present.  The value depends on the type.
static const string GEOJSON_COORDINATES = "coordinates";

/// @brief parse GeoJSON Type
bool GeoParser::parseGeoJSONType(const AqlValue geoJSON) {
  if (!geoJSON.isObject()) {
    return 0;
  }

  VPackSlice slice = geoJSON.slice();
  VPackSlice type = slice.get("type");
 
  if (!type.isString()) {
    return GeoParser::GEOJSON_UNKNOWN;
  }

  const string& typeString = type.copyString();

  if (GEOJSON_TYPE_POINT == typeString) {
    return GeoParser::GEOJSON_POINT;
  } else if (GEOJSON_TYPE_LINESTRING == typeString) {
    return GeoParser::GEOJSON_LINESTRING;
  } else if (GEOJSON_TYPE_POLYGON == typeString) {
    return GeoParser::GEOJSON_POLYGON;
  } else if (GEOJSON_TYPE_MULTI_POINT == typeString) {
    return GeoParser::GEOJSON_MULTI_POINT;
  } else if (GEOJSON_TYPE_MULTI_LINESTRING == typeString) {
    return GeoParser::GEOJSON_MULTI_LINESTRING;
  } else if (GEOJSON_TYPE_MULTI_POLYGON == typeString) {
    return GeoParser::GEOJSON_MULTI_POLYGON;
  } else if (GEOJSON_TYPE_GEOMETRY_COLLECTION == typeString) {
    return GeoParser::GEOJSON_GEOMETRY_COLLECTION;
  }
  return GeoParser::GEOJSON_UNKNOWN;
};

/// @brief parse GeoJSON Polygon Type
bool GeoParser::parseGeoJSONTypePolygon(const AqlValue geoJSON) {
  if (!geoJSON.isObject()) {
    return 0;
  }

  VPackSlice slice = geoJSON.slice();
  VPackSlice type = slice.get("type");
 
  if (!type.isString()) {
    return GeoParser::GEOJSON_UNKNOWN;
  }

  const string& typeString = type.copyString();

  // verify type
  if (GEOJSON_TYPE_POLYGON != typeString) {
    return 0;
  }
  return 1;
};

// begin helper functions
static double ParseDouble(const string& str) {
  char* end_ptr = NULL;
  double value = strtod(str.c_str(), &end_ptr);
  // CHECK(end_ptr && *end_ptr == 0) << ": str == \"" << str << "\"";
  return value;
}

void ParseLatLngs(string const& str, vector<S2LatLng>* latlngs) {
  vector<pair<string, string> > p;
  DictionaryParse(str, &p);
  latlngs->clear();
  for (int i = 0; i < p.size(); ++i) {
    latlngs->push_back(S2LatLng::FromDegrees(ParseDouble(p[i].first),
          ParseDouble(p[i].second)));
  }
}

void ParsePoints(string const& str, vector<S2Point>* vertices) {
  vector<S2LatLng> latlngs;
  ParseLatLngs(str, &latlngs);
  vertices->clear();
  for (int i = 0; i < latlngs.size(); ++i) {
    LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "latlngs: " << latlngs[i];
    // FORMAT [50.8300000, 6.9175000]
    vertices->push_back(latlngs[i].ToPoint());
  }
}

static S2Loop* MakeLoop(string const& str) {
  vector<S2Point> vertices;
  ParsePoints(str, &vertices);
  return new S2Loop(vertices);
}

// create a s2 polygon function
S2Polygon* MakePolygon(string const& str) {
  vector<string> loop_strs;
  SplitStringUsing(str, ";", &loop_strs);
  vector<S2Loop*> loops;

  for (int i = 0; i < loop_strs.size(); ++i) {
    S2Loop* loop = MakeLoop(loop_strs[i]);
    loop->Normalize();
    loops.push_back(loop);
  }
  return new S2Polygon(&loops);  // Takes ownership.
}
// end helper functions

/// @brief create and return polygon
S2Polygon* GeoParser::parseGeoJSONPolygon(const AqlValue geoJSON) {
  const string sa = "50.8300:6.9175, 50.8044:6.9381, 50.7752:6.9949,  50.7927:7.0271,  50.8189:7.0209, 50.8365,6.9755";
  // TODO #1: verify polygon values
  // VerifyPolygon(geoJSON);

  // TODO #2: build polygon
  return MakePolygon(sa);
};
