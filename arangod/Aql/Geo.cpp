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

#include "Geo.h"
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

class S2Polygon;

bool Geo::contains(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  // verify if object is in geojson format
  if (!gp.parseGeoJSONType(geoJSONA) || !gp.parseGeoJSONType(geoJSONB)) {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON format.");
  }

  if (gp.parseGeoJSONTypePolygon(geoJSONA) && gp.parseGeoJSONTypePolygon(geoJSONB)) {
    return containsPolygon(geoJSONA, geoJSONB);
  } else {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON format.");
  }
};

/// @brief function EQUALS
bool Geo::containsPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  bool result;
  
  // equals polygon
  S2Polygon* polyA = gp.parseGeoJSONPolygon(geoJSONA);
  S2Polygon* polyB = gp.parseGeoJSONPolygon(geoJSONB);

  result = polyA->Contains(polyB);
  return result;
};

bool Geo::equals(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  // verify if object is in geojson format
  if (!gp.parseGeoJSONType(geoJSONA) || !gp.parseGeoJSONType(geoJSONB)) {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON format.");
  }

  if (gp.parseGeoJSONTypePolygon(geoJSONA) && gp.parseGeoJSONTypePolygon(geoJSONB)) {
    return equalsPolygon(geoJSONA, geoJSONB);
  } else {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON format.");
  }
};

bool Geo::equalsPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB) {
};
