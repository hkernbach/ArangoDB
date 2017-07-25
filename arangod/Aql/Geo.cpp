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

/// @brief function CONTAINS
bool Geo::containsPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  bool result;
  
  // equals polygon
  S2Polygon* polyA = gp.parseGeoJSONPolygon(geoJSONA);
  S2Polygon* polyB = gp.parseGeoJSONPolygon(geoJSONB);

  result = polyA->Contains(polyB);
  return result;
};

/// @brief function CONTAINS
bool Geo::polygonContainsPoint(const AqlValue geoJSONA, const AqlValue geoJSONB) {
/*  scoped_ptr<S2Polygon> a(S2Testing::MakePolygon(a_str));
  EXPECT_TRUE(a->VirtualContainsPoint(S2Testing::MakePoint(b_str)))
    << " " << a_str << " did not contain " << b_str;*/
};

bool Geo::equals(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  // verify if object is in geojson format
  if (!gp.parseGeoJSONType(geoJSONA) || !gp.parseGeoJSONType(geoJSONB)) {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON type.");
  }

  if (gp.parseGeoJSONTypePolygon(geoJSONA) && gp.parseGeoJSONTypePolygon(geoJSONB)) {
    return equalsPolygon(geoJSONA, geoJSONB);
  } else {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON polygon.");
  }
};

bool Geo::equalsPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB) {
};

double Geo::distance(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  if (gp.parseGeoJSONTypePoint(geoJSONA) && gp.parseGeoJSONTypePolygon(geoJSONB)) {
    return distancePointToPolygon(geoJSONA, geoJSONB);
  } else if (gp.parseGeoJSONTypePolygon(geoJSONA) && gp.parseGeoJSONTypePoint(geoJSONB)) {
    return distancePointToPolygon(geoJSONB, geoJSONA);
  } else {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON polygon.");
  }
};

double Geo::distancePointToPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  S2Polygon* poly = gp.parseGeoJSONPolygon(geoJSONB);
  S2Point point = gp.parseGeoJSONPoint(geoJSONA);
//  S1Angle distance = S1Angle(poly.Project(point), point);
//  return distance.degrees();
};

// Returns all available points within a Polygon
AqlValue Geo::pointsInPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  // geoJSONA: type MultiPoints
  // geoJSONB: type Polygon

  // Parse Polygon and MultiPoint
  S2Polygon* poly = gp.parseGeoJSONPolygon(geoJSONB);
  vector<S2Point> multiPoint = gp.parseGeoJSONMultiPoint(geoJSONA);

  // 1. Calculate the center of the polygon
  // 2. Calculate the highest radius available
  // 3. Get all points within that circle using the geo index
  // 4. Check each point if it exists in the range of the given polygon
  // 5. Put all fitting points into a new AqlValue MultiPoint Value and return

};
