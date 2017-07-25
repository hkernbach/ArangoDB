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
  VPackSlice coordinates = slice.get("coordinates");
 
  if (!type.isString()) {
    return GeoParser::GEOJSON_UNKNOWN;
  }

  if (!coordinates.isArray()) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON coordinates format.");
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

/// @brief parse GeoJSON Point Type
bool GeoParser::parseGeoJSONTypePoint(const AqlValue geoJSON) {
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
  if (GEOJSON_TYPE_POINT != typeString) {
    return 0;
  }
  return 1;
};

// parse geojson coordinates into s2 points
void ParsePoints(const AqlValue geoJSON, vector<S2Point>* vertices) {
  vector<S2LatLng> latlngs;
  vertices->clear();

  VPackSlice slice = geoJSON.slice();
  VPackSlice coordinates = slice.get("coordinates");

  if (coordinates.isArray()) {
    for (auto const& coordinate : VPackArrayIterator(coordinates)) {
      vertices->push_back(S2LatLng::FromDegrees(coordinate.at(0).getNumber<double>(),
        coordinate.at(1).getNumber<double>()).ToPoint());
    }
  }
}

static S2Loop* MakeLoop(const AqlValue geoJSON) {
  vector<S2Point> vertices;
  ParsePoints(geoJSON, &vertices);
  return new S2Loop(vertices);
}

// create a s2 polygon function
S2Polygon* MakePolygon(const AqlValue geoJSON) {
  vector<S2Loop*> loops;

  S2Loop* loop = MakeLoop(geoJSON);
  loop->Normalize();
  loops.push_back(loop);
  return new S2Polygon(&loops);  // Takes ownership.
}

// create a s2 point function
S2Point MakePoint(const AqlValue geoJSON) {
  // TODO: Just a placeholder function to compile currently
  double x = 1.2;
  double y = 3.4;
  double z = 5.6;
  return S2Point(x, y, z).Normalize();
  // TODO: Just a placeholder function to compile currently
}

// create a std vector filled with points (multipoint)
vector<S2Point> MakeMultiPoint(const AqlValue geoJSON) {
  vector<S2Point> multiPoint;
  ParsePoints(geoJSON, &multiPoint);

  return multiPoint;
}

/// @brief create and return polygon
S2Polygon* GeoParser::parseGeoJSONPolygon(const AqlValue geoJSON) {
  // TODO #1: verify polygon values
  // VerifyPolygon(geoJSON);

  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "result second: " << "s";
  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "result second: " << "s";
  // TODO #2: build polygon
  return MakePolygon(geoJSON);
};

/// @brief create multipoint vector
S2Point GeoParser::parseGeoJSONPoint(const AqlValue geoJSON) {
  return MakePoint(geoJSON);
};

/// @brief create multipoint vector
vector<S2Point> GeoParser::parseGeoJSONMultiPoint(const AqlValue geoJSON) {
  // TODO #1: verify points values
  // each points -> VerifyPoint(geoJSON);


  // TODO #2: build vector containing points
  return MakeMultiPoint(geoJSON);
};
