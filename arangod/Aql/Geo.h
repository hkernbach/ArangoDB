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

#ifndef ARANGOD_AQL_GEO_H
#define ARANGOD_AQL_GEO_H 1

#include "Basics/Common.h"
#include "Aql/AqlValue.h"

namespace arangodb {

namespace velocypack {
  class Builder;
  class Slice;
}

namespace aql {

class Geo {
 public:
  //explicit Geo(arangodb::velocypack::Slice const&);

  virtual ~Geo() {}

 public:
   bool equals(const AqlValue geoJSONA, const AqlValue geoJSONB);
   bool equalsPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB);
   bool contains(const AqlValue geoJSONA, const AqlValue geoJSONB);
   bool containsPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB);
   bool polygonContainsPoint(const AqlValue geoJSONA, const AqlValue geoJSONB);
   double distance(const AqlValue geoJSONA, const AqlValue geoJSONB);
   double distancePointToPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB);
   AqlValue pointsInPolygon(const AqlValue geoJSONA, const AqlValue geoJSONB);
 private:

};

}  // namespace aql
}  // namespace arangodb

#endif
