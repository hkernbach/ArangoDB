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

#include "Transaction/Helpers.h"
#include "Transaction/Methods.h"
#include "VocBase/LogicalCollection.h"
#include "VocBase/ManagedDocumentResult.h"
#include "Utils/CollectionNameResolver.h"
#include "MMFiles/MMFilesAqlFunctions.h"
#include "MMFiles/MMFilesGeoIndex.h"

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

using namespace arangodb;
using namespace arangodb::aql;
using namespace arangodb::basics;

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

// Case A: check if given points are within the polygon
AqlValue Geo::helperPointsInPolygon(const AqlValue collectionName, const AqlValue geoJSONA, transaction::Methods* trx) {
  GeoParser gp;
  // verify if object is in geojson format
  if (!gp.parseGeoJSONType(geoJSONA)) {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON type!!.");
  }

  if (gp.parseGeoJSONTypePolygon(geoJSONA)) {
    return pointsInPolygon(collectionName, geoJSONA, trx);
  } else {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON polygon.");
  }
};

/*
// Case B: check if given points are within the polygon
AqlValue Geo::helperPointsInPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  // verify if object is in geojson format
  if (!gp.parseGeoJSONType(geoJSONA) || !gp.parseGeoJSONType(geoJSONB)) {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON type.");
  }

  if (gp.parseGeoJSONTypePoint(geoJSONA) && gp.parseGeoJSONTypePolygon(geoJSONB)) {
    return pointsInPolygon(geoJSONA, geoJSONB);
  } else {
    // TODO: add invalid geo json error
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_GRAPH_INVALID_GRAPH, "Invalid GeoJSON polygon.");
  }
};
*/

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

/// @brief Load geoindex for collection name
arangodb::MMFilesGeoIndex* getGeoIndex(
    transaction::Methods* trx, TRI_voc_cid_t const& cid,
    std::string const& collectionName) {
  // NOTE:
  // Due to trx lock the shared_index stays valid
  // as long as trx stays valid.
  // It is save to return the Raw pointer.
  // It can only be used until trx is finished.
  trx->addCollectionAtRuntime(cid, collectionName);

  auto document = trx->documentCollection(cid);

  if (document == nullptr) {
    THROW_ARANGO_EXCEPTION_FORMAT(TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND,
                                  "'%s'", collectionName.c_str());
  }

  arangodb::MMFilesGeoIndex* index = nullptr;

  for (auto const& idx : document->getIndexes()) {
    if (idx->type() == arangodb::Index::TRI_IDX_TYPE_GEO1_INDEX ||
        idx->type() == arangodb::Index::TRI_IDX_TYPE_GEO2_INDEX) {
      index = static_cast<arangodb::MMFilesGeoIndex*>(idx.get());
      break;
    }
  }

  if (index == nullptr) {
    THROW_ARANGO_EXCEPTION_PARAMS(TRI_ERROR_QUERY_GEO_INDEX_MISSING,
                                  collectionName.c_str());
  }

  trx->pinData(cid);

  return index;
}

AqlValue buildGeoResult(transaction::Methods* trx,
                               LogicalCollection* collection,
                               GeoCoordinates* cors,
                               TRI_voc_cid_t const& cid
                               ) {
  if (cors == nullptr) {
    return AqlValue(arangodb::basics::VelocyPackHelper::EmptyArrayValue());
  }

  size_t const nCoords = cors->length;
  if (nCoords == 0) {
    GeoIndex_CoordinatesFree(cors);
    return AqlValue(arangodb::basics::VelocyPackHelper::EmptyArrayValue());
  }

  struct geo_coordinate_distance_t {
    geo_coordinate_distance_t(double distance, DocumentIdentifierToken token)
        : _distance(distance), _token(token) {}

    double _distance;
    DocumentIdentifierToken _token;
  };

  std::vector<geo_coordinate_distance_t> distances;

  try {
    distances.reserve(nCoords);

    for (size_t i = 0; i < nCoords; ++i) {
      distances.emplace_back(geo_coordinate_distance_t(
          cors->distances[i],
          arangodb::MMFilesGeoIndex::toDocumentIdentifierToken(
              cors->coordinates[i].data)));
    }
  } catch (...) {
    GeoIndex_CoordinatesFree(cors);
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

  GeoIndex_CoordinatesFree(cors);

  // sort result by distance
  std::sort(distances.begin(), distances.end(),
            [](geo_coordinate_distance_t const& left,
               geo_coordinate_distance_t const& right) {
              return left._distance < right._distance;
            });

  try {
    ManagedDocumentResult mmdr;
    transaction::BuilderLeaser builder(trx);
    builder->openArray();
    /*
    if (!attributeName.empty()) {
      // We have to copy the entire document
      for (auto& it : distances) {
        VPackObjectBuilder docGuard(builder.get());
        builder->add(attributeName, VPackValue(it._distance));
        if (collection->readDocument(trx, it._token, mmdr)) {
          VPackSlice doc(mmdr.vpack());
          for (auto const& entry : VPackObjectIterator(doc)) {
            std::string key = entry.key.copyString();
            if (key != attributeName) {
              builder->add(key, entry.value);
            }
          }
        }
      }

    } else {
    */
    for (auto& it : distances) {
      if (collection->readDocument(trx, it._token, mmdr)) {
        mmdr.addToBuilder(*builder.get(), true);
      }
    }
   // }
    builder->close();
    return AqlValue(builder.get());
  } catch (...) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
}

// Case A: Returns all available points within a Polygon
AqlValue Geo::pointsInPolygon(const AqlValue collectionName, const AqlValue geoJSONA, transaction::Methods* trx) {
  GeoParser gp;
  // geoJSONA: type Polygon

  // Parse Polygon
  S2Polygon* poly = gp.parseGeoJSONPolygon(geoJSONA);

  // 1. Calculate the center of the polygon
  S2Point center = poly->GetCentroid();
  // 2. Calculate the highest radius available
  double radius = 0.0;
  double calculatedDistance = 0.0;

  vector<S2Point> pointsOfPolygon = gp.parseGeoJSONMultiPoint(geoJSONA);
  for ( auto &i : pointsOfPolygon ) {
    calculatedDistance = S1Angle(center, i).radians();
    if (calculatedDistance > radius) {
      radius = calculatedDistance;
    }
    // expected
    // calculatedDistance = new S1Angle(S2Point const& center, S2Point const& i).radians();
    LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "calc: " << calculatedDistance;
  }
  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "radius: " << radius;
  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "TEST";

  S2LatLng a = S2LatLng::FromDegrees(50.937531, 6.960279).Normalized(); // Cologne
  S2LatLng b = S2LatLng::FromDegrees(50.737430, 7.098207).Normalized(); // Bonn
  S1Angle x = a.GetDistance(b);
  double xx = a.GetDistance(b).radians();
  double xxx = a.GetDistance(b).degrees();

  // double x = S1Angle(a, b).degrees();
  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "distance x: " << x;
  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "distance xx: " << xx;
  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "distance xxx: " << xxx;

  // TEST 2

  double blabla = S2LatLng::FromDegrees(50.937531, 6.960279).GetDistance(S2LatLng::FromDegrees(50.737430, 7.098207)).degrees();
  double blablax = S2LatLng::FromDegrees(6.960279, 50.937531).GetDistance(S2LatLng::FromDegrees(7.098207, 50.737430)).degrees();
  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "BLABLA 1: " << blabla;
  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "BLABLA 2: " << blablax;

  // TEST 2 END

  LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "TEST";

  // extract limit
  int64_t limitValue = 100;

  // read collection name TODO
  std::string const collectionNameString(collectionName.slice().copyString());

  // GET INDEX -- if no index available -> full scan
  TRI_voc_cid_t cid = trx->resolver()->getCollectionIdLocal(collectionNameString);
  arangodb::MMFilesGeoIndex* index = getGeoIndex(trx, cid, collectionNameString);

    TRI_ASSERT(index != nullptr);    // ?? found ??
    TRI_ASSERT(trx->isPinned(cid));  // ?? ?? ??

    // 3. Get all points within that circle using the geo index
    GeoCoordinates* cors = index->nearQuery(
        trx, S2LatLng(center).lat().degrees(), S2LatLng(center).lng().degrees(), static_cast<size_t>(limitValue));

    AqlValue indexedPoints = buildGeoResult(trx, index->collection(), cors, cid);

  // S2EdgeUtil::GetDistance(x, a, b).radians()

  // 4. Check each point if it exists in the range of the given polygon
  // 5. Put all fitting points into a new AqlValue MultiPoint Value and return
};

// Case B: Check if points are within polygon
/*
AqlValue Geo::pointsInPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB) {
  GeoParser gp;
  // geoJSONA: type MultiPoints
  // geoJSONB: type Polygon

  // Parse Polygon and MultiPoint
  S2Polygon* poly = gp.parseGeoJSONPolygon(geoJSONB);
  vector<S2Point> multiPoint = gp.parseGeoJSONMultiPoint(geoJSONA);

  // 1. Calculate the center of the polygon
  S2Point center = poly.GetCentroid();
  // 2. Calculate the highest radius available
  double distance = 0.0;
  double calculatedDistance = 0.0;

  vector<S2Point> pointsOfPolygon = gp.parseGeoJSONMultiPoint(geoJSONB);
  for ( auto &i : pointsOfPolygon ) {
    calculatedDistance = new S1Angle(S2Point const& center, S2Point const& i).radians();
    LOG_TOPIC(ERR, arangodb::Logger::FIXME) << "distance: " << calculatedDistance;
  }


  // S2EdgeUtil::GetDistance(x, a, b).radians()

  // 3. Get all points within that circle using the geo index
  // 4. Check each point if it exists in the range of the given polygon
  // 5. Put all fitting points into a new AqlValue MultiPoint Value and return

};
*/
