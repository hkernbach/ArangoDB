////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2017 ArangoDB GmbH, Cologne, Germany
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
/// @author Jan Steemann
////////////////////////////////////////////////////////////////////////////////

#include "ApplicationFeatures/JemallocFeature.h"

#include "Basics/FileUtils.h"
#include "Logger/Logger.h"
#include "ProgramOptions/ProgramOptions.h"

#include <iostream>

using namespace arangodb;
using namespace arangodb::basics;
using namespace arangodb::options;

char JemallocFeature::_staticPath[PATH_MAX];

JemallocFeature::JemallocFeature(
    application_features::ApplicationServer* server)
    : ApplicationFeature(server, "Jemalloc") {
  setOptional(false);
  requiresElevatedPrivileges(false);
}

void JemallocFeature::collectOptions(std::shared_ptr<ProgramOptions> options) {
  options->addSection("vm", "Virtual memory");

  options->addOption("--vm.resident-limit", "resident limit in bytes",
                     new UInt64Parameter(&_residentLimit));

  options->addOption("--vm.path", "path to the directory for vm files",
                     new StringParameter(&_path));
}

void JemallocFeature::validateOptions(std::shared_ptr<ProgramOptions>) {
  static uint64_t MIN_LIMIT = 512 * 1024 * 1024;

  if (0 < _residentLimit && _residentLimit < MIN_LIMIT) {
    LOG_TOPIC(INFO, Logger::MEMORY) << "vm.resident-limit of " << _residentLimit
                                    << " is to small, using " << MIN_LIMIT;

    _residentLimit = MIN_LIMIT;
  }

  if (_path.empty()) {
    _path = ".";
  }

  FileUtils::makePathAbsolute(_path);
  FileUtils::normalizePath(_path);

  _path += TRI_DIR_SEPARATOR_STR;

  LOG_TOPIC(INFO, Logger::MEMORY) << "vm.resident-limit = " << _residentLimit
                                  << ", vm.path = " << _path;
}

extern "C" void adb_jemalloc_set_limit(size_t limit, char const* path);

void JemallocFeature::prepare() {
  *_staticPath = '\0';

  if (0 < _residentLimit) {
    if (_path.empty()) {
      strncat(_staticPath, "./", PATH_MAX);
    } else {
      strncat(_staticPath, _path.c_str(), PATH_MAX);
    }

    if (!FileUtils::isDirectory(_staticPath)) {
      if (!FileUtils::createDirectory(_staticPath, 0700)) {
        LOG_TOPIC(FATAL, Logger::MEMORY)
            << "cannot create directory '" << _staticPath
            << "' for VM files: " << strerror(errno);
        FATAL_ERROR_EXIT();
      }
    }

    adb_jemalloc_set_limit(_residentLimit, _staticPath);
  }
}
